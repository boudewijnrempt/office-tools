#!/bin/bash

astyle --style=linux --indent=spaces=4 --brackets=linux --break-blocks --convert-tabs --unpad=paren -M79 src/*.cpp
astyle --style=linux --indent=spaces=4 --brackets=linux --break-blocks --convert-tabs --unpad=paren -M79 src/*.h

astyle --style=linux --indent=spaces=4 --brackets=linux --break-blocks --convert-tabs --unpad=paren -M79 -r tests/*/*.cpp
astyle --style=linux --indent=spaces=4 --brackets=linux --break-blocks --convert-tabs --unpad=paren -M79 -r tests/*/*.h
