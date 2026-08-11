CREATE TABLE IF NOT EXISTS tof_monitor (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    run_number INT,
    hname VARCHAR(32),
    bin_index INT,
    bin_content FLOAT,
    hist_type INT,     -- 1:TH1F, 2:TH2F, 3:TProfile
    n_bins_x INT,
    x_min FLOAT,
    x_max FLOAT,
    n_bins_y INT,
    y_min FLOAT,
    y_max FLOAT,
    INDEX (hname, timestamp)
);

