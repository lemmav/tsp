FROM nginx:1.15.8

# Override the default nginx configuration file
RUN rm /etc/nginx/conf.d/default.conf
RUN apt update && apt install -y iputils-ping && apt install -y dnsutils
COPY nginx.conf /etc/nginx/nginx.conf 
COPY static /static
