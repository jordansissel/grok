#!/bin/sh

/bin/ls test*.sh | xargs -P1 -n1 sh
