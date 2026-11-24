-- ProSlides database schema
-- Design goals:
--   * keep the FreeShow JSON document as the source of truth (`json` columns)
--   * maintain normalized projections for fast queries, sorting and FTS5 search
--   * all timestamps are epoch milliseconds (FreeShow convention)

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS meta (
    key   TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

-- ---------------------------------------------------------------------------
-- Shows
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS shows (
    id          TEXT PRIMARY KEY,
    name        TEXT NOT NULL,
    file_path   TEXT,
    origin      TEXT,
    category    TEXT,
    is_private  INTEGER NOT NULL DEFAULT 0,
    locked      INTEGER NOT NULL DEFAULT 0,
    created     INTEGER NOT NULL,
    modified    INTEGER,
    used        INTEGER,
    tags        TEXT,
    meta_key    TEXT,
    json        TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_shows_name ON shows(name COLLATE NOCASE);
CREATE INDEX IF NOT EXISTS idx_shows_modified ON shows(modified DESC);

-- ---------------------------------------------------------------------------
-- Slides (normalized projection of Show.slides)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS slides (
    id          TEXT PRIMARY KEY,
    show_id     TEXT NOT NULL REFERENCES shows(id) ON DELETE CASCADE,
    group_name  TEXT,
    title       TEXT,
    plain_text  TEXT NOT NULL DEFAULT '',
    notes       TEXT NOT NULL DEFAULT '',
    musical_key TEXT,
    color       TEXT,
    background  TEXT,
    position    INTEGER NOT NULL DEFAULT 0,
    locked      INTEGER NOT NULL DEFAULT 0,
    json        TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_slides_show ON slides(show_id, position);
CREATE INDEX IF NOT EXISTS idx_slides_group ON slides(show_id, group_name);

-- ---------------------------------------------------------------------------
-- Layouts and their slide references (FreeShow Layout.slides: SlideData[])
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS layouts (
    id           TEXT PRIMARY KEY,
    show_id      TEXT NOT NULL REFERENCES shows(id) ON DELETE CASCADE,
    name         TEXT NOT NULL,
    notes        TEXT NOT NULL DEFAULT '',
    is_active    INTEGER NOT NULL DEFAULT 0,
    position     INTEGER NOT NULL DEFAULT 0,
    json         TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_layouts_show ON layouts(show_id, position);

CREATE TABLE IF NOT EXISTS layout_slides (
    layout_id    TEXT NOT NULL REFERENCES layouts(id) ON DELETE CASCADE,
    slide_id     TEXT NOT NULL REFERENCES slides(id) ON DELETE CASCADE,
    position     INTEGER NOT NULL,
    disabled     INTEGER NOT NULL DEFAULT 0,
    transition   TEXT,
    background   TEXT,
    duration_ms  INTEGER,
    PRIMARY KEY (layout_id, position)
);
CREATE INDEX IF NOT EXISTS idx_layout_slides_slide ON layout_slides(slide_id);

-- ---------------------------------------------------------------------------
-- Media descriptors (Show.media)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS media (
    id          TEXT PRIMARY KEY,
    show_id     TEXT REFERENCES shows(id) ON DELETE CASCADE,
    name        TEXT,
    path        TEXT NOT NULL,
    type        TEXT NOT NULL DEFAULT 'media',
    video_type  TEXT,
    muted       INTEGER NOT NULL DEFAULT 0,
    loop        INTEGER NOT NULL DEFAULT 0,
    json        TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_media_path ON media(path);
CREATE INDEX IF NOT EXISTS idx_media_show ON media(show_id);

-- ---------------------------------------------------------------------------
-- Projects / service flow and their show references
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS projects (
    id          TEXT PRIMARY KEY,
    name        TEXT NOT NULL,
    notes       TEXT NOT NULL DEFAULT '',
    parent      TEXT NOT NULL DEFAULT 'root',
    created     INTEGER NOT NULL,
    modified    INTEGER,
    used        INTEGER,
    archived    INTEGER NOT NULL DEFAULT 0,
    json        TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_projects_parent ON projects(parent, name COLLATE NOCASE);

CREATE TABLE IF NOT EXISTS project_items (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    project_id  TEXT NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    position    INTEGER NOT NULL,
    show_id     TEXT,
    layout_id   TEXT,
    section     TEXT,
    name        TEXT,
    item_type   TEXT,
    color       TEXT,
    notes       TEXT,
    played      INTEGER NOT NULL DEFAULT 0,
    json        TEXT NOT NULL,
    UNIQUE (project_id, position)
);
CREATE INDEX IF NOT EXISTS idx_project_items_project ON project_items(project_id, position);

-- ---------------------------------------------------------------------------
-- Folders (project tree)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS folders (
    id          TEXT PRIMARY KEY,
    name        TEXT NOT NULL,
    parent      TEXT NOT NULL DEFAULT 'root',
    created     INTEGER NOT NULL,
    modified    INTEGER,
    json        TEXT NOT NULL
);

-- ---------------------------------------------------------------------------
-- Timeline actions (FreeShow Timeline: time = ms, duration = seconds)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS timeline_actions (
    id          TEXT PRIMARY KEY,
    owner_type  TEXT NOT NULL,  -- 'show' | 'layout' | 'slide' | 'project'
    owner_id    TEXT NOT NULL,
    time_ms     INTEGER NOT NULL,
    duration_s  REAL NOT NULL DEFAULT 0,
    name        TEXT,
    type        TEXT,
    color       TEXT,
    json        TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_timeline_owner ON timeline_actions(owner_type, owner_id, time_ms);

-- ---------------------------------------------------------------------------
-- Full text search (FTS5)
--   unicode61 remove_diacritics 2 lets "toi" match "tôi", "yeu" match "yêu".
--   content='slides' keeps a single copy of the text (contentless external table).
-- ---------------------------------------------------------------------------
CREATE VIRTUAL TABLE IF NOT EXISTS slides_fts USING fts5(
    title,
    group_name,
    plain_text,
    notes,
    musical_key,
    content='slides',
    content_rowid='rowid',
    tokenize='unicode61 remove_diacritics 2'
);

-- Triggers keep FTS in sync with the slides table.
CREATE TRIGGER IF NOT EXISTS slides_ai AFTER INSERT ON slides BEGIN
    INSERT INTO slides_fts(rowid, title, group_name, plain_text, notes, musical_key)
    VALUES (new.rowid, new.title, new.group_name, new.plain_text, new.notes, new.musical_key);
END;

CREATE TRIGGER IF NOT EXISTS slides_ad AFTER DELETE ON slides BEGIN
    INSERT INTO slides_fts(slides_fts, rowid, title, group_name, plain_text, notes, musical_key)
    VALUES ('delete', old.rowid, old.title, old.group_name, old.plain_text, old.notes, old.musical_key);
END;

CREATE TRIGGER IF NOT EXISTS slides_au AFTER UPDATE ON slides BEGIN
    INSERT INTO slides_fts(slides_fts, rowid, title, group_name, plain_text, notes, musical_key)
    VALUES ('delete', old.rowid, old.title, old.group_name, old.plain_text, old.notes, old.musical_key);
    INSERT INTO slides_fts(rowid, title, group_name, plain_text, notes, musical_key)
    VALUES (new.rowid, new.title, new.group_name, new.plain_text, new.notes, new.musical_key);
END;

-- ---------------------------------------------------------------------------
-- Scripture (FreeShow .fsb content + full-text index)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS bibles (
    id       TEXT PRIMARY KEY,
    name     TEXT NOT NULL,
    metadata TEXT,
    copyright TEXT
);
CREATE VIRTUAL TABLE IF NOT EXISTS verses_fts USING fts5(
    bible_id UNINDEXED,
    book,
    chapter UNINDEXED,
    verse UNINDEXED,
    text,
    tokenize='unicode61 remove_diacritics 2'
);

-- ---------------------------------------------------------------------------
-- Overlays / templates (kept as JSON, FreeShow Config/*.json compatible)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS overlays (
    id       TEXT PRIMARY KEY,
    name     TEXT NOT NULL,
    category TEXT,
    color    TEXT,
    modified INTEGER,
    json     TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS templates (
    id       TEXT PRIMARY KEY,
    name     TEXT NOT NULL,
    category TEXT,
    color    TEXT,
    modified INTEGER,
    json     TEXT NOT NULL
);

-- Rebuild FTS index if the tables already existed but the index was empty.
INSERT INTO slides_fts(slides_fts) VALUES ('rebuild');
