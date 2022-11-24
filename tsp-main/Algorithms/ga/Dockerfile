FROM  frolvlad/alpine-gxx:latest
WORKDIR /usr/src/ga
COPY . .
RUN apk update && apk upgrade && apk add bash && g++ -o ga 'TSP GA.cpp' -I .
CMD ./ga