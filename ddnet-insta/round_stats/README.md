# post and consume round stats from python

This is a small example of how you can use the round stats the ddnet-insta server can publish.
There is a list of config variables that allow you to send a result of the game in different formats
to different destinations when a round ends. One of these formats is json and one of these destinations is
a http endpoint. This example is especially about that. Which allows you to build for example your own
website that tracks ddnet-insta games as they happen.

In your autoexec_server.cfg you need to configure that you want json round stats
posted to an http endpoint

```
# autoexec_server.cfg

# you need this if you want to test without https on your local machine
http_allow_insecure 1

# swap the url our with your hostname if you have one
sv_round_stats_http_endpoints "http:/127.0.0.1:8000/stats"

# just example mode and scorelimit change to whatever you want
sv_gametype dm
sv_scorelimit 1
```

Then in your terminal run this or any other equivalent command to install the
python flask package

```bash
# this only works on macOS and linux!

python3 -m venv venv
source venv/bin/activate
pip install flask
```

Then you can start the backend like this

```
python3 example_backend_server.py
```

And then start a ddnet-insta server and play a round till round end. At that point you should receive the round data.
If you copied the config exactly it will be dm and you need to at least make one kill to win.
At that point you will see the round stats being printed by the example python backend.
If that works you can go from there and do with the data whatever you want.
