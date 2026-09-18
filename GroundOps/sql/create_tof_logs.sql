CREATE TABLE IF NOT EXISTS tof_logs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    run_number INT,
    timestamp_ms BIGINT,      -- uint64_t timestamp_ms
    level TINYINT,            -- uint8_t level (0~5) 
    level_str VARCHAR(16),    -- 'INFO', 'CRIT'
    component VARCHAR(16),    -- Fixed 16-byte component
    message TEXT,             -- Arbitrary-length text
    INDEX (timestamp),
    INDEX (level),
    INDEX (component)
);
