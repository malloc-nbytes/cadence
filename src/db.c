#include "db.h"
#include "array.h"
#include "global.h"
#include "sqlite3.h"
#include "str.h"

#include <stdio.h>

static int
db_exec(const char *sql)
{
        char *error;
        int rc;

        error = NULL;
        rc = sqlite3_exec(g_config.db, sql, NULL, NULL, &error);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "database error: %s\n",
                        error != NULL ? error : sqlite3_errmsg(g_config.db));
                sqlite3_free(error);
                return 0;
        }

        return 1;
}

void
track_destroy(track *tr)
{
        str_destroy(&tr->artist);
        str_destroy(&tr->album);
        str_destroy(&tr->name);
        str_destroy(&tr->path);
}

void
playlist_destroy(playlist *pl)
{
        for (size_t i = 0; i < pl->tracks.len; ++i)
                track_destroy(&pl->tracks.data[i]);
        array_free(pl->tracks);
        str_destroy(&pl->name);
}

static void
track_from_stmt(sqlite3_stmt *stmt, track *tr)
{
        const unsigned char *artist;
        const unsigned char *album;
        const unsigned char *name;
        const unsigned char *path;

        artist = sqlite3_column_text(stmt, 3);
        album = sqlite3_column_text(stmt, 4);
        name = sqlite3_column_text(stmt, 5);
        path = sqlite3_column_text(stmt, 7);

        *tr = (track) {
                .id = sqlite3_column_int64(stmt, 0),
                .artist_id = sqlite3_column_int64(stmt, 1),
                .album_id = sqlite3_column_int64(stmt, 2),
                .artist = str_from((const char *)artist),
                .album = str_from((const char *)album),
                .name = str_from((const char *)name),
                .path = str_from((const char *)path),
                .duration_ms = sqlite3_column_int64(stmt, 6),
                .file_mtime = sqlite3_column_int64(stmt, 8),
                .file_size = sqlite3_column_int64(stmt, 9),
                .added_at = sqlite3_column_int64(stmt, 10),
                .last_played_at = sqlite3_column_int64(stmt, 11),
                .track_number = (unsigned)sqlite3_column_int(stmt, 12),
                .disc_number = (unsigned)sqlite3_column_int(stmt, 13),
                .play_count = (unsigned)sqlite3_column_int(stmt, 14),
        };
}

static int
track_stmt_load_all(sqlite3_stmt *stmt, track_ar *tracks)
{
        int rc;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                track tr;

                track_from_stmt(stmt, &tr);
                array_append(*tracks, tr);
        }

        if (rc != SQLITE_DONE) {
                fprintf(stderr, "could not load tracks: %s\n",
                        sqlite3_errmsg(g_config.db));
                return 0;
        }

        return 1;
}

int
db_track_load(sqlite3_int64 id, track *tr)
{
        static const char sql[] =
                "SELECT tracks.id, tracks.artist_id, "
                "       COALESCE(tracks.album_id, 0), "
                "       artists.name, COALESCE(albums.name, ''), "
                "       tracks.name, tracks.duration_ms, tracks.path, "
                "       COALESCE(tracks.file_mtime, 0), "
                "       COALESCE(tracks.file_size, 0), "
                "       tracks.added_at, "
                "       COALESCE(tracks.last_played_at, 0), "
                "       COALESCE(tracks.track_number, 0), "
                "       tracks.disc_number, tracks.play_count "
                "FROM tracks "
                "JOIN artists ON artists.id = tracks.artist_id "
                "LEFT JOIN albums ON albums.id = tracks.album_id "
                "WHERE tracks.id = ?1;";
        sqlite3_stmt *stmt;
        int rc;

        *tr = (track){0};

        rc = sqlite3_prepare_v2(g_config.db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "could not prepare track query: %s\n",
                        sqlite3_errmsg(g_config.db));
                return 0;
        }

        rc = sqlite3_bind_int64(stmt, 1, id);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "could not bind track id: %s\n",
                        sqlite3_errmsg(g_config.db));
                sqlite3_finalize(stmt);
                return 0;
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
                track_from_stmt(stmt, tr);
                sqlite3_finalize(stmt);
                return 1;
        }

        if (rc != SQLITE_DONE)
                fprintf(stderr, "could not load track: %s\n",
                        sqlite3_errmsg(g_config.db));

        sqlite3_finalize(stmt);
        return 0;
}

int
db_tracks_load(track_ar *tracks)
{
        static const char sql[] =
                "SELECT tracks.id, tracks.artist_id, "
                "       COALESCE(tracks.album_id, 0), "
                "       artists.name, COALESCE(albums.name, ''), "
                "       tracks.name, tracks.duration_ms, tracks.path, "
                "       COALESCE(tracks.file_mtime, 0), "
                "       COALESCE(tracks.file_size, 0), "
                "       tracks.added_at, "
                "       COALESCE(tracks.last_played_at, 0), "
                "       COALESCE(tracks.track_number, 0), "
                "       tracks.disc_number, tracks.play_count "
                "FROM tracks "
                "JOIN artists ON artists.id = tracks.artist_id "
                "LEFT JOIN albums ON albums.id = tracks.album_id "
                "ORDER BY artists.name, albums.name, tracks.disc_number, "
                "         tracks.track_number, tracks.name;";
        sqlite3_stmt *stmt;
        int rc;

        *tracks = array_empty_explicit(track_ar);

        rc = sqlite3_prepare_v2(g_config.db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "could not prepare tracks query: %s\n",
                        sqlite3_errmsg(g_config.db));
                return 0;
        }

        rc = track_stmt_load_all(stmt, tracks);
        sqlite3_finalize(stmt);

        if (!rc) {
                for (size_t i = 0; i < tracks->len; ++i)
                        track_destroy(&tracks->data[i]);
                array_free(*tracks);
        }

        return rc;
}

int
db_playlist_load(sqlite3_int64 id, playlist *pl)
{
        static const char playlist_sql[] =
                "SELECT id, name, created_at, updated_at "
                "FROM playlists WHERE id = ?1;";
        static const char tracks_sql[] =
                "SELECT tracks.id, tracks.artist_id, "
                "       COALESCE(tracks.album_id, 0), "
                "       artists.name, COALESCE(albums.name, ''), "
                "       tracks.name, tracks.duration_ms, tracks.path, "
                "       COALESCE(tracks.file_mtime, 0), "
                "       COALESCE(tracks.file_size, 0), "
                "       tracks.added_at, "
                "       COALESCE(tracks.last_played_at, 0), "
                "       COALESCE(tracks.track_number, 0), "
                "       tracks.disc_number, tracks.play_count "
                "FROM playlist_tracks "
                "JOIN tracks ON tracks.id = playlist_tracks.track_id "
                "JOIN artists ON artists.id = tracks.artist_id "
                "LEFT JOIN albums ON albums.id = tracks.album_id "
                "WHERE playlist_tracks.playlist_id = ?1 "
                "ORDER BY playlist_tracks.position;";
        sqlite3_stmt *playlist_stmt;
        sqlite3_stmt *tracks_stmt;
        const unsigned char *name;
        int rc;

        *pl = (playlist){0};

        rc = sqlite3_prepare_v2(g_config.db, playlist_sql, -1,
                &playlist_stmt, NULL);
        if (rc != SQLITE_OK)
                goto error;

        if (sqlite3_bind_int64(playlist_stmt, 1, id) != SQLITE_OK)
                goto error_playlist_stmt;

        rc = sqlite3_step(playlist_stmt);
        if (rc != SQLITE_ROW)
                goto error_playlist_stmt;

        name = sqlite3_column_text(playlist_stmt, 1);
        *pl = (playlist) {
                .id = sqlite3_column_int64(playlist_stmt, 0),
                .name = str_from((const char *)name),
                .created_at = sqlite3_column_int64(playlist_stmt, 2),
                .updated_at = sqlite3_column_int64(playlist_stmt, 3),
                .tracks = array_empty(),
        };
        sqlite3_finalize(playlist_stmt);

        rc = sqlite3_prepare_v2(g_config.db, tracks_sql, -1,
                &tracks_stmt, NULL);
        if (rc != SQLITE_OK)
                goto error_playlist;

        if (sqlite3_bind_int64(tracks_stmt, 1, id) != SQLITE_OK) {
                sqlite3_finalize(tracks_stmt);
                goto error_playlist;
        }

        rc = track_stmt_load_all(tracks_stmt, &pl->tracks);
        sqlite3_finalize(tracks_stmt);
        if (!rc)
                goto error_playlist;

        return 1;

error_playlist_stmt:
        sqlite3_finalize(playlist_stmt);
error:
        fprintf(stderr, "could not load playlist: %s\n",
                sqlite3_errmsg(g_config.db));
        return 0;

error_playlist:
        playlist_destroy(pl);
        *pl = (playlist){0};
        fprintf(stderr, "could not load playlist tracks: %s\n",
                sqlite3_errmsg(g_config.db));
        return 0;
}

int
db_playlists_load(playlist_ar *playlists)
{
        static const char sql[] =
                "SELECT id, name, created_at, updated_at "
                "FROM playlists ORDER BY name;";
        sqlite3_stmt *stmt;
        int rc;

        *playlists = array_empty_explicit(playlist_ar);

        rc = sqlite3_prepare_v2(g_config.db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "could not prepare playlists query: %s\n",
                        sqlite3_errmsg(g_config.db));
                return 0;
        }

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                const unsigned char *name;
                playlist pl;

                name = sqlite3_column_text(stmt, 1);
                pl = (playlist) {
                        .id = sqlite3_column_int64(stmt, 0),
                        .name = str_from((const char *)name),
                        .created_at = sqlite3_column_int64(stmt, 2),
                        .updated_at = sqlite3_column_int64(stmt, 3),
                        .tracks = array_empty(),
                };
                array_append(*playlists, pl);
        }

        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE)
                return 1;

        fprintf(stderr, "could not load playlists: %s\n",
                sqlite3_errmsg(g_config.db));

        for (size_t i = 0; i < playlists->len; ++i)
                playlist_destroy(&playlists->data[i]);
        array_free(*playlists);

        return 0;
}

int
db_init(const char *path)
{
        static const char schema[] =
                "BEGIN;"
                ""
                "CREATE TABLE IF NOT EXISTS artists ("
                "        id   INTEGER PRIMARY KEY,"
                "        name TEXT NOT NULL COLLATE NOCASE UNIQUE"
                ");"
                ""
                "CREATE TABLE IF NOT EXISTS albums ("
                "        id INTEGER   PRIMARY KEY,"
                "        artist_id    INTEGER REFERENCES artists(id)"
                "                             ON DELETE SET NULL,"
                "        name         TEXT NOT NULL COLLATE NOCASE,"
                "        release_year INTEGER"
                "                             CHECK (release_year IS NULL"
                "                                    OR release_year BETWEEN 1 AND 9999),"
                "        UNIQUE (artist_id, name, release_year)"
                ");"
                ""
                "CREATE TABLE IF NOT EXISTS tracks ("
                "        id             INTEGER PRIMARY KEY,"
                "        artist_id      INTEGER NOT NULL"
                "                               REFERENCES artists(id)"
                "                               ON DELETE RESTRICT,"
                "        album_id       INTEGER REFERENCES albums(id)"
                "                               ON DELETE SET NULL,"
                "        name           TEXT NOT NULL COLLATE NOCASE,"
                "        duration_ms    INTEGER NOT NULL"
                "                               CHECK (duration_ms >= 0),"
                "        path           TEXT NOT NULL UNIQUE,"
                "        track_number   INTEGER"
                "                               CHECK (track_number IS NULL"
                "                                      OR track_number > 0),"
                "        disc_number    INTEGER NOT NULL DEFAULT 1"
                "                               CHECK (disc_number > 0),"
                "        file_mtime     INTEGER,"
                "        file_size      INTEGER"
                "                               CHECK (file_size IS NULL OR file_size >= 0),"
                "        added_at       INTEGER NOT NULL"
                "                               DEFAULT (unixepoch()),"
                "        last_played_at INTEGER,"
                "        play_count     INTEGER NOT NULL DEFAULT 0"
                "                               CHECK (play_count >= 0)"
                ");"
                ""
                "CREATE INDEX IF NOT EXISTS tracks_artist_id_idx"
                "        ON tracks (artist_id);"
                ""
                "CREATE INDEX IF NOT EXISTS tracks_album_id_idx"
                "        ON tracks (album_id);"
                ""
                "CREATE TABLE IF NOT EXISTS playlists ("
                "        id         INTEGER PRIMARY KEY,"
                "        name       TEXT NOT NULL COLLATE NOCASE UNIQUE,"
                "        created_at INTEGER NOT NULL DEFAULT (unixepoch()),"
                "        updated_at INTEGER NOT NULL DEFAULT (unixepoch())"
                ");"
                ""
                "CREATE TABLE IF NOT EXISTS playlist_tracks ("
                "        playlist_id INTEGER NOT NULL"
                "                            REFERENCES playlists(id)"
                "                            ON DELETE CASCADE,"
                "        track_id    INTEGER NOT NULL"
                "                    REFERENCES tracks(id)"
                "                    ON DELETE CASCADE,"
                "        position    INTEGER NOT NULL CHECK (position >= 0),"
                "        added_at    INTEGER NOT NULL DEFAULT (unixepoch()),"
                "        PRIMARY KEY (playlist_id, position)"
                ");"
                ""
                "CREATE INDEX IF NOT EXISTS playlist_tracks_track_id_idx"
                "        ON playlist_tracks (track_id);"
                ""
                "COMMIT;";

        if (sqlite3_open_v2(path, &g_config.db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
                fprintf(stderr, "could not open database '%s': %s\n",
                        path, g_config.db != NULL ? sqlite3_errmsg(g_config.db) : "unnknown error");
                sqlite3_close(g_config.db);
                g_config.db = NULL;
                return 0;
        }

        if (!db_exec("PRAGMA foreign_keys = ON;")
            || !db_exec("PRAGMA journal_mode = WAL;")
            || !db_exec(schema)) {
                db_close();
                return 0;
        }

        return 1;
}

void
db_close(void)
{
        if (g_config.db != NULL) {
                sqlite3_close(g_config.db);
                g_config.db = NULL;
        }
}
