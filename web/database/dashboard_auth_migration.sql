CREATE TABLE IF NOT EXISTS dashboard_users
(
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    username VARCHAR(100) NOT NULL,
    display_name VARCHAR(150) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    status ENUM('ACTIVE','DISABLED') NOT NULL DEFAULT 'ACTIVE',
    last_login DATETIME NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_dashboard_users_username (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS user_gateway_access
(
    user_id INT UNSIGNED NOT NULL,
    gateway_id VARCHAR(32) NOT NULL,
    access_role ENUM('VIEWER','OPERATOR','ADMIN') NOT NULL DEFAULT 'VIEWER',
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, gateway_id),
    KEY idx_gateway_access_gateway (gateway_id),
    CONSTRAINT fk_gateway_access_user
        FOREIGN KEY (user_id) REFERENCES dashboard_users(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
