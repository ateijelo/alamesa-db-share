# AlaMesa DB Share

This is a desktop app designed for sharing AlaMesa's db file on the local network.

It has two components:

 * A highly-asynchronous HTTP server based on [qhttp](https://github.com/azadkuh/qhttp),
   with support added for file transfers and range requests (for resuming
   interrupted downloads). It can easily handle hundreds of simultaneous
   connections without breaking a sweat.
 * A call to `avahi-publish` on Linux and `dns-sd` on macOS to advertise its
   own service on the local network. If you must run it on Windows, you can
   manually advertise the existence of the service from another Mac or Linux
   computer. If you don't have any Mac or Linux computer around, I'm sorry for
   your loss.

There's some more info in [this post](http://www.ateijelo.com/blog/2016/08/07/alamesa-db-share) in my blog.
