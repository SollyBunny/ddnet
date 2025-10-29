# Solly's Client

A combination of

* [DDNet](https://github.com/ddnet/ddnet/)
* [TClient](https://github.com/sjrc6/TaterClient-ddnet/)
* [DDNet Insta](https://github.com/ddnet-insta/ddnet-insta/)
* ~~[Infclass Client](https://github.com/infclass/infclass-client)~~

## Download

You can download nightly builds from [here](https://nightly.link/SollyBunny/ddnet/workflows/fast-build/master?preview)

If you need 32bit, or something for a similarly archaic or weird system, compile it yourself by following the [guide](https://github.com/ddnet/ddnet/?tab=readme-ov-file#cloning)

## Diff

If you don't trust me or are just curious as to what is changed between this client and it's upstreams you can perform a diff

Here are the links for your convenience

* [DDNet](https://github.com/ddnet/ddnet/compare/master...SollyBunny:ddnet:master)
* [TClient](https://github.com/sjrc6/TaterClient-ddnet/compare/master...SollyBunny:ddnet:master)
* [DDNet Insta](https://github.com/ddnet-insta/ddnet-insta/compare/master...SollyBunny:ddnet:master)
* ~~[Infclass Client](https://github.com/infclass/infclass-client/compare/infc-0.2.0-on-19.1...SollyBunny:ddnet:master)~~

### Webhook Tutorial

Solly's Client has the ability to send all console lines to a remote HTTP server.

It is up to users to not do anything illegal such as any input modification

Here is a few uses for this:

* Link up with external moderation tools
* Shutdown your computer when you swear

Set the destination with `sc_webhook_address` (eg `http://localhost:8080`)

If you use an insecure server (HTTP) you can allow it with `http_allow_insecure 1`

The API is just plain text delimated by new lines

When one or more console lines are sent, the text you will get looks like this

```
0
bun bun: Hello
1
broadcast You're stinky
0
server: You're stinky
```

0 is local console, 1 is rcon.

Future consoles will be 2 and 3, etc.

You can send special messages with `webhook_command ...`, which should be preferred over parsing local console as that may have injection attacks, this will appear as console -1.

You can send back messages in the same format to have them play on the client
