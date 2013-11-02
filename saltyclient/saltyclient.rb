require 'socket'

hostname = 'localhost'
port = 1337

s = TCPSocket.open(hostname,port);
puts s.gets #burn welcome message
while line = s.gets
    puts "player 1: " + line.chop
    puts "player 2: " + s.gets.chop
end
s.close
