# deployed to https://salty-dawn-69757.herokuapp.com/
# manage on heroku at : https://dashboard.heroku.com/apps/salty-dawn-69757/settings

require 'json'
require "sinatra"
require 'shotgun'
require 'active_support/all'
require "active_support/core_ext"
require 'rake'
require 'particle'  # require particle gem to talk to the photon
require 'twilio-ruby'  # connect to twilio


# enable sessions for this project
enable :sessions


# Load environment variables using Dotenv. If a .env file exists, it will
# set environment variables from that file (useful for dev environments)
configure :development do
  require 'dotenv'
  Dotenv.load
end


# CREATE A CLient
client = Twilio::REST::Client.new ENV["TWILIO_ACCOUNT_SID"], ENV["TWILIO_AUTH_TOKEN"]
particle_client = Particle::Client.new( access_token: ENV['PARTICLE_ACCESS_TOKEN'] )


# Use this method to check if your ENV file is set up
get "/" do
  "Hello world!"
end

get "/from" do
  ENV["TWILIO_FROM"]
end

# Test sending an SMS
# change the to to your number 
get "/send_sms" do

  client.account.messages.create(
    :from => "+14152002424",
    :to => "+12067130783",
    :body => "Hey there. This is a test"
  )

  "Sent message"
  
end


# Hook this up to your Webhook for SMS/MMS through the console
get '/incoming_sms' do

  session["counter"] ||= 0
  count = session["counter"]
  
  sender = params[:From] || ""
  body = params[:Body] || ""
  body = body.downcase.strip
  body_toint = body.to_i
  
  event_data = ""

  print session["counter"]
  print session["last_context"]
  
  if body.include? "off" or body.include? "quiet"
    event_data = "notifoff : #{ body }"
    message = "Notifications have been turned off."
  elsif body.include? "on"
    event_data = "notifon : #{ body }" 
    message = "Notifications are now on."
  elsif body.starts_with? "set"
    event_data = "settime : #{ body }" 
    message = "How many days do you want the timer to last? Depending on the food, 5-15 days is usually best. (Reply with just a number.)"
  elsif body_toint < 4 and body_toint > 0
    event_data = "#{ body }"    # Removing to extract int more easily - "numdays_short:"
    message = "Timer set for #{ body } days. Fun fact, savethefood.com says most food can be stored more than 3 days. To reset timer, type any number >3 or if you want to keep as is, just place me in the fridge."
  elsif body_toint >= 4 and body_toint <= 30
    event_data = "#{ body }"  # Removing to extract int more easily - numdays_OK:
    message = "Great! Timer is for #{body} days. Go ahead and place me in the fridge."
  elsif body_toint > 30
    event_data = "numdays_long : #{ body }"
    message = "Whoaaaa, #{ body } days?! My batteries wont last that long. Lets stay under a month. Enter a number 1-30."
  elsif body.include? "bye"
    event_data = "bye:#{ body }"
    message = get_bye
  else
    event_data = "error:#{ body }"
    message = get_error
  end

  particle_client.publish(name: "smart_food/sms/incoming/#{sender}", data: event_data)

  session["counter"] += 1
  
  twiml = Twilio::TwiML::Response.new do |r|
    r.Message message
  end

  content_type 'text/xml'

  twiml.text

end

error 401 do 
  "Sorry, I didn't get that."
end

def get_context
  session["last_context"] || nil
end 

BYE = ["Cheerio.", "Tata for now.", "Hasta la vista, baby"]

def get_bye
    return BYE.sample
end

ERROR_SAMPLES = ["Sorry, I did not get that. Type SET to adjust the timer. Otherwise, put me in the fridge.", "Hmmm, je ne comprende pas. Please enter a value 1-30 or place me in the fridge.", "WHAT DID YOU SAY TO ME?! Oh, sorry. Wrong chat. But could you repeat that?"]

def get_error
    return ERROR_SAMPLES.sample
end


