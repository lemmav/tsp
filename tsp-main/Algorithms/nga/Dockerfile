FROM  frolvlad/alpine-gxx:latest
WORKDIR /usr/src/nga
COPY . .
RUN apk update && apk upgrade && apk add bash && g++ -o nga 'Neighbour+GA.cpp' -I .
CMD ./nga