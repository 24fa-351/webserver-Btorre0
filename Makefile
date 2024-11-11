CC = gcc
CFLAGS = -Wall -Wextra -g -lpthread
TARGET = webserver
SRCS = webserver.c handle.c http_message.c
HEADERS = handle.h http_message.h

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS)

run: $(TARGET)
	./$(TARGET) -p 8080

check: $(TARGET)
	./$(TARGET) -p 8090 & \
	SERVER_PID=$$!; \
	sleep 1; \
	\
	echo "Testing /stats endpoint"; \
	curl -s http://localhost:8090/stats || echo "Server not running"; \
	\
	echo "Testing /static endpoint"; \
	mkdir -p static; \
	echo "Sample static file content" > static/test.txt; \
	curl -s http://localhost:8090/static/test.txt || echo "Failed to retrieve static file"; \
	\
	echo "Testing /calc endpoint: para a=5 and b=10"; \
	curl -s "http://localhost:8090/calc?a=5&b=10" || echo "Calculation endpoint failed"; \
	\
	echo "Testing 404 Not Found for nonexistent path"; \
	curl -s -o /dev/null -w "%{http_code}" http://localhost:8090/nonexistent | grep 404 || echo "404 handler failed"; \
	\
	echo "All tests completed."; \
	kill $$SERVER_PID

clean:
	rm -f $(TARGET)
	rm -f static/test.txt