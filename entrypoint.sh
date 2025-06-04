#!/bin/bash
IMAGE_NAME="candles"

OUTPUT=$(docker run $IMAGE_NAME)


echo "<!DOCTYPE html>
<html>
<head>
<title>candle tests</title>
<style>
    body {
        width: 35em;
        margin: 0 auto;
        font-family: Tahoma, Verdana, Arial, sans-serif;
    }
</style>
</head>
<body>
<h1>test results</h1>
<p>${OUTPUT}</p>

</body>
</html>
" > /usr/share/nginx/html/index.html

exec nginx -g 'daemon off;'