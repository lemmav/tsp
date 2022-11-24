FROM  frolvlad/alpine-gxx:latest
WORKDIR /usr/src/bnn
COPY . .
RUN apk update && apk upgrade && apk add bash && g++ -o bnn 'BetterNearestNeighbourSearch.cpp' -I .
CMD ./bnn