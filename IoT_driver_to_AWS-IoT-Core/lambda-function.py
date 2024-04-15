import boto3
import time
import datetime

def lambda_handler(event, context):
    client = boto3.client('dynamodb')

    response = client.put_item(
        TableName = 'esp32Data',
        Item = {

            'timestamp': {'S': str(datetime.datetime.now())},
            'timestamp_esp32': {'S': str(datetime.datetime.utcfromtimestamp(event['epochtime']) + datetime.timedelta(milliseconds=event['milliseconds']))},
            'size': {'N': str(event['size'])},
            'package': {'S': event['package']},
            # 'epochtime': {'N': str(event['epochtime'])},
            'milliseconds': {'N': str(event['milliseconds'])},
            'delta': {'S': str(datetime.datetime.now() - datetime.datetime.utcfromtimestamp(event['epochtime']) - datetime.timedelta(milliseconds=event['milliseconds']))}
        }
    )

    return 0
