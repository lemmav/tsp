FROM  frolvlad/alpine-gxx:latest
WORKDIR /usr/src/bnb
COPY . .
RUN apk update && apk upgrade && apk add bash && g++ -o bnb 'B&B.cpp' -I .
CMD ./bnb