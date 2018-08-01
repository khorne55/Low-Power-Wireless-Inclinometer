import sys
import logging
import rds_config
import pymysql
import datetime
#rds settings
rds_host  = "host_name"
name = rds_config.db_username
password = rds_config.db_password
db_name = rds_config.db_name


logger = logging.getLogger()
logger.setLevel(logging.INFO)

#connecting to the database
try:
    conn = pymysql.connect(rds_host, user=name, passwd=password, db=db_name, connect_timeout=5)
except:
    logger.error("ERROR: Unexpected error: :( Could not connect to MySql instance.")
    sys.exit()

logger.info("SUCCESS: Connection to RDS mysql instance succeeded")
def handler(event, context):
    """
    This function fetches content from mysql RDS instance
    """

    item_count = 0

    with conn.cursor() as cur:
		#converting the hex payload to String
        data=bytearray.fromhex(event['data']).decode()
		#seperating the data into Roll and Pitch
        datarray=data.split()
		#converting back to the original decimal format
        roll=float(datarray[0])/10
        pitch=float(datarray[1])/10
		#converting the timestamp to a date time format
        date=datetime.datetime.fromtimestamp(int(event['time'])).strftime('%Y-%m-%d %H:%M:%S')
		#inserting the data into the db
        message='insert into tilt_data (timestamp, deviceid, roll, pitch, snr, avgsnr) values("{}", "{}", "{}", "{}", "{}", "{}")'.format(date,event['device'],roll,pitch,event['snr'],event['avgSnr'])
        cur.execute(message)
        conn.commit()
        cur.execute("select * from tilt_data")
        for row in cur:
            item_count += 1
            logger.info(row)

    return "Added %s items from RDS MySQL table" %(event['data'])
