#!/bin/bash
scp -rp ../facetracker finstad@lux:
ssh finstad@lux "cd facetracker && ./build.sh && DISPLAY=:0 ./main"
