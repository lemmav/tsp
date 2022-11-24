FROM  frolvlad/alpine-gxx:latest
WORKDIR /usr/src/nn
COPY . .
RUN apk update && apk upgrade && apk add bash && g++ -o nn 'NearestNeighborSearch.cpp' -I .
CMD ./nn