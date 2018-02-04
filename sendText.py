import subprocess, os
from twilio.rest import Client

result = subprocess.check_output('hostname -I', shell=True).rstrip()

account_sid = "AC42b741fa59c37d330a022dce748b5f58" # This is the Twilio Account info from Twilio
auth_token = "38aa8bbf4577e69abc3dd52f1ea68506" # This is the Twilio Account "Password"

client = Client(account_sid, auth_token)

message = client.api.account.messages.create(
			to='+14038808432', # This is the phone number that the Pi will send to. Trial version only lets you text the number you used to authenticate.
			from_='+15873552673', # This is the phone number twilio gives you.
			body="The RPI IP Address is: {} ".format(result)) # This is the message.
