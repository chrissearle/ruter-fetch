A simple setup to put my local T-Bane on a 5110 LCD with arduino

## script

Node code to run on a web server - this just fetches the info and does the json parsing.
Install it with cron and redirect output to a static file for serving over http

The URL polled is:

    reisapi.ruter.no/StopVisit/GetDepartures/3010360

## arduino

The arduino code for putting the data onto the LCD.

This should run on either the uno or micro and with the full ethernet shield or
with a smaller W5100 connector.

Uses arduino's EthernetClient and [HttpClient](https://github.com/amcewen/HttpClient)
to talk to the internet.

Uses [u8g2](https://github.com/olikraus/u8g2) to talk to the screen.