CREATE TABLE IF NOT EXISTS queue (
                                url TEXT PRIMARY KEY UNIQUE,
                                graph TEXT NOT NULL,
                                algorithm TEXT NOT NULL,
                                requesttime TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS progress (
                                url TEXT PRIMARY KEY UNIQUE ,
                                graph TEXT NOT NULL,
                                algorithm TEXT NOT NULL,
                                starttime TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS result (
                                url TEXT PRIMARY KEY UNIQUE,
                                answer REAL NOT NULL,
                                algorithm TEXT NOT NULL,
                                executiontime TEXT NOT NULL);