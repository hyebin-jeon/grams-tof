CREATE TABLE IF NOT EXISTS tof_graphs (
    id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    run_number  INT UNSIGNED    NOT NULL,
    gname       VARCHAR(16)     NOT NULL,
    point_index INT UNSIGNED    NOT NULL,
    x_val       DOUBLE          NOT NULL,
    y_val       DOUBLE          NOT NULL,
    PRIMARY KEY (id),
    INDEX idx_tof_graphs_run_gname (run_number, gname)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS tof_parameters (
    id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    run_number  INT UNSIGNED    NOT NULL,
    pname       VARCHAR(16)     NOT NULL,
    value       DOUBLE          NOT NULL,
    PRIMARY KEY (id),
    INDEX idx_tof_parameters_run_pname (run_number, pname)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
