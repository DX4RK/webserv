FROM debian:bullseye

RUN apt update && apt install -y build-essential

WORKDIR /app
COPY . .

RUN make

EXPOSE 8080

CMD ["./webserv"]
