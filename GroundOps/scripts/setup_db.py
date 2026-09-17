import mysql.connector
import os
import sys
import logging
import subprocess
import glob

# --- FIXED LOGGING CONFIG ---
log_filename = "setup_db.log"
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s %(levelname)s: %(message)s',
    datefmt='[%Y-%m-%d %H:%M:%S]',
    handlers=[
        logging.FileHandler(log_filename),
        logging.StreamHandler(sys.stdout)
    ]
)

#db_user = os.getenv("TOF_MYSQL_USER")
#db_pass = os.getenv("TOF_MYSQL_PASS")
#db_name = os.getenv("TOF_MYSQL_DB")
db_user = os.getenv("PGRAMS_MYSQL_USER")
db_pass = os.getenv("PGRAMS_MYSQL_PASSWD")
db_name = os.getenv("PGRAMS_MYSQL_DB")
glib_path = os.getenv("GLIB")

def setup():
    if not all([db_user, db_pass, db_name]):
        logging.error("Environment variables TOF_MYSQL_USER, TOF_MYSQL_PASS, or TOF_MYSQL_DB are not set.")
        sys.exit(1)

    try:
        logging.info(f"Starting database setup for '{db_name}'...")
        
        conn = mysql.connector.connect(
            host="localhost",
            user="root",
            unix_socket="/var/run/mysqld/mysqld.sock"
        )
        cursor = conn.cursor()

        # 1. Create Database
        cursor.execute(f"CREATE DATABASE IF NOT EXISTS {db_name};")
        
        # 2. Create User and Grant Permissions
        for host in ['localhost', '127.0.0.1']:
            cursor.execute(f"CREATE USER IF NOT EXISTS '{db_user}'@'{host}' IDENTIFIED BY '{db_pass}';")
            cursor.execute(f"GRANT ALL PRIVILEGES ON {db_name}.* TO '{db_user}'@'{host}';")
        
        cursor.execute("FLUSH PRIVILEGES;")
        logging.info(f"Database '{db_name}' and user '{db_user}' permissions set.")

        # 3. Import All Table Schemas in ground/sql/
        sql_dir = os.path.join(glib_path, "ground/sql") if glib_path else None
        if sql_dir and os.path.exists(sql_dir):
            sql_files = glob.glob(os.path.join(sql_dir, "*.sql"))
            for sql_file in sql_files:
                logging.info(f"Importing schema file: {os.path.basename(sql_file)}...")
                cmd = f"mysql -u {db_user} -p'{db_pass}' -D {db_name} < {sql_file}"
                subprocess.run(cmd, shell=True, check=True)
                logging.info(f"Schema from {os.path.basename(sql_file)} applied successfully.")
        else:
            logging.warning("SQL schema directory not found.")

    except Exception as e:
        logging.error(f"Setup failed: {e}")
        sys.exit(1)
    finally:
        if 'conn' in locals() and conn.is_connected():
            cursor.close()
            conn.close()

if __name__ == "__main__":
    setup()
