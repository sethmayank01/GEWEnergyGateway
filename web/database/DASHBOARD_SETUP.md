# Dashboard authentication setup

1. Run `dashboard_auth_migration.sql` once in the energy database.
2. Generate a password hash on the server (do not store a plain-text password):

   ```powershell
   C:\xampp\php\php.exe -r "echo password_hash('CHANGE-THIS-PASSWORD', PASSWORD_DEFAULT), PHP_EOL;"
   ```

3. Create the user with the generated hash:

   ```sql
   INSERT INTO dashboard_users (username, display_name, password_hash)
   VALUES ('customer1', 'Customer 1', 'PASTE_PASSWORD_HASH_HERE');
   ```

4. Assign only the gateways that user may view:

   ```sql
   INSERT INTO user_gateway_access (user_id, gateway_id, access_role)
   SELECT id, 'GEW000001', 'VIEWER'
   FROM dashboard_users
   WHERE username = 'customer1';
   ```

Repeat step 4 for additional gateways belonging to the same user. A user without an assignment can log in but sees no gateway data.

Deploy `login.php`, `logout.php`, `includes/`, `dashboard.php`, `dashboard/`, `assets/`, and the protected `api/status.php`, `api/readings.php`, and `api/trends.php` together. The configured web path is `/energy/`.
