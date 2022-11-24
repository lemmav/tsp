from app import server, create_db

if __name__ == "__main__":

    server.run(host='0.0.0.0', port=8000)
# from app import server

# if __name__ == "__main__":

#     server.run(host='0.0.0.0', port=8000)
#docker run -p=8000:8000 -v /home/lemmav/diplom/dbdir:/flask gm:2
