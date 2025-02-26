# Diretórios
SRC_DIR = source
INCLUDE_DIR = include
OBJ_DIR = object
LIB_DIR = lib
BIN_DIR = binary

# Compilador e flags
CC = gcc
CFLAGS = -I$(INCLUDE_DIR) -Wall -Wextra -g

# Bibliotecas e objetos
LIBTABLE = $(LIB_DIR)/libtable.a
LIBTABLE_OBJS = $(OBJ_DIR)/block.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o

# Dependências do cliente e do servidor
CLIENT_OBJS = $(OBJ_DIR)/client_hashtable.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/client_network.o $(OBJ_DIR)/htmessages.pb-c.o $(OBJ_DIR)/message-private.o $(OBJ_DIR)/stats.o $(OBJ_DIR)/zoo_helper.o
SERVER_OBJS = $(OBJ_DIR)/server_hashtable.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/client_network.o $(OBJ_DIR)/server_skeleton.o $(OBJ_DIR)/server_network.o $(OBJ_DIR)/htmessages.pb-c.o $(OBJ_DIR)/message-private.o $(OBJ_DIR)/stats.o $(OBJ_DIR)/zoo_helper.o

# Alvo principal
all: $(SRC_DIR)/htmessages.pb-c.c libtable client_hashtable server_hashtable

# Construção da biblioteca estática libtable.a
libtable: $(LIBTABLE)
$(LIBTABLE): $(LIBTABLE_OBJS)
	@mkdir -p $(LIB_DIR)
	ar -rcs $(LIBTABLE) $(LIBTABLE_OBJS)

# Compilação do programa client_hashtable
client_hashtable: $(CLIENT_OBJS) $(LIBTABLE)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/client_hashtable -L$(LIB_DIR) -ltable -lprotobuf-c -lzookeeper_mt

# Compilação dos objetos do cliente
$(OBJ_DIR)/client_hashtable.o: $(SRC_DIR)/client_hashtable.c $(INCLUDE_DIR)/client_stub.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/client_stub.o: $(SRC_DIR)/client_stub.c $(INCLUDE_DIR)/client_stub.h $(INCLUDE_DIR)/htmessages.pb-c.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/client_network.o: $(SRC_DIR)/client_network.c $(INCLUDE_DIR)/client_network.h $(INCLUDE_DIR)/client_stub.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/htmessages.pb-c.o: $(SRC_DIR)/htmessages.pb-c.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/message-private.o: $(SRC_DIR)/message-private.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/stats.o: $(SRC_DIR)/stats.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/zoo_helper.o: $(SRC_DIR)/zoo_helper.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compilação do programa server_hashtable
server_hashtable: $(SERVER_OBJS) $(LIBTABLE)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/server_hashtable -L$(LIB_DIR) -ltable -lprotobuf-c -lzookeeper_mt

# Compilação dos objetos do servidor
$(OBJ_DIR)/server_hashtable.o: $(SRC_DIR)/server_hashtable.c $(INCLUDE_DIR)/server_skeleton.h $(INCLUDE_DIR)/server_network.h 
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/server_skeleton.o: $(SRC_DIR)/server_skeleton.c $(INCLUDE_DIR)/server_skeleton.h $(INCLUDE_DIR)/htmessages.pb-c.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/server_network.o: $(SRC_DIR)/server_network.c $(INCLUDE_DIR)/server_network.h $(INCLUDE_DIR)/server_skeleton.h
	$(CC) $(CFLAGS) -c $< -o $@


# Geração dos arquivos htmessages.pb-c.c e htmessages.pb-c.h
$(SRC_DIR)/htmessages.pb-c.c: htmessages.proto
	@mkdir -p $(SRC_DIR) $(INCLUDE_DIR)
	protoc --c_out=. htmessages.proto
	mv htmessages.pb-c.c $(SRC_DIR)
	mv htmessages.pb-c.h $(INCLUDE_DIR)


# Alvo para limpar arquivos compilados
clean:
	rm -f $(OBJ_DIR)/client_hashtable.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/client_network.o $(OBJ_DIR)/htmessages.pb-c.o
	rm -f $(OBJ_DIR)/server_hashtable.o $(OBJ_DIR)/server_skeleton.o $(OBJ_DIR)/server_network.o $(OBJ_DIR)/message-private.o
	rm -f $(OBJ_DIR)/stats.o $(OBJ_DIR)/htmessages.pb-c.o $(OBJ_DIR)/zoo_helper.o
	rm -rf $(LIBTABLE) $(BIN_DIR)/client_hashtable $(BIN_DIR)/server_hashtable