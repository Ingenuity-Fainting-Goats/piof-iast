#!/bin/bash

docker stop piof-iast-instance
docker rm piof-iast-instance
docker build -t piof-iast .

docker run -d --name piof-iast-instance piof-iast

docker exec -i -t piof-iast-instance "/opt/piof-iast/scripts/tests.sh"
docker exec -i -t piof-iast-instance "/bin/bash"