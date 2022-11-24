FROM  python:3.8.15-buster

WORKDIR /var/usr/checkdb
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY . .
CMD python3 checkdb.py