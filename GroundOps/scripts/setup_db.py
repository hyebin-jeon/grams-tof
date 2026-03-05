import mysql.connector
import os
import sys

db_user = os.getenv("TOF_MYSQL_USER")
db_pass = os.getenv("TOF_MYSQL_PASS")
db_name = os.getenv("TOF_MYSQL_DB")

def setup():
    try:
        conn = mysql.connector.connect(
            host="localhost",
            user="root",
            unix_socket="/var/run/mysqld/mysqld.sock" 
        )
        cursor = conn.cursor()

        # 1. Create Database
        cursor.execute(f"CREATE DATABASE IF NOT EXISTS {db_name};")

        # 2. Create User for both Local Socket and TCP Network
        for host in ['localhost', '127.0.0.1']:
            cursor.execute(f"""
                CREATE USER IF NOT EXISTS '{db_user}'@'{host}'
                IDENTIFIED WITH mysql_native_password BY '{db_pass}';
            """)
       
        # 3. Grant Permissions for both
        cursor.execute(f"GRANT ALL PRIVILEGES ON {db_name}.* TO '{db_user}'@'{host}';")
        cursor.execute("FLUSH PRIVILEGES;")

        print(f"Success: Database '{db_name}' and user '{db_user}' are ready.")
        
    except Exception as e:
        print(f"Setup failed: {e}")
        sys.exit(1)
    finally:
        if 'conn' in locals() and conn.is_connected():
            cursor.close()
            conn.close()

if __name__ == "__main__":
    setup()
