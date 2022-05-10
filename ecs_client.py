import socket

def createConnection():

    return socket.create_connection(("localhost", 7070))

def createContainer(containerName):

    sock = createConnection()

    print("\n\nElastic Container Service - ECS Client: Connection established on port 7070")

    try:

        print("\n\nElastic Container Service - ECS Client: Trying request...")

        clientRequest = ("create " + containerName).encode()

        sock.sendall(clientRequest)

    finally:

        print("\n\nElastic Container Service - ECS Client: Request made succesfully")

        print("\n\nElastic Container Service - ECS Client: Closing connection...")

        sock.close()

        print("\n\nElastic Container Service - ECS Client: Connection closed succesfully")

def listContainers():

    sock = createConnection()

    print("\n\nElastic Container Service - ECS Client: Connection established on port 7070")

    try:

        print("\n\nElastic Container Service - ECS Client: Trying request...")

        clientRequest = ("list").encode()

        sock.sendall(clientRequest)

    finally:

        print("\n\nElastic Container Service - ECS Client: Request made succesfully")

        print("\n\nElastic Container Service - ECS Client: Closing connection...")

        print("\n\nElastic Container Service - ECS Client: Connection closed succesfully")

        sock.close()

def stopContainer(containerName):

    sock = createConnection()

    print("\n\nElastic Container Service - ECS Client: Connection established on port 7070")

    try:

        print("\n\nElastic Container Service - ECS Client: Trying request...")

        clientRequest = ("stop " + containerName).encode()

        sock.sendall(clientRequest)

    finally:

        print("\n\nElastic Container Service - ECS Client: Request made succesfully")

        print("\n\nElastic Container Service - ECS Client: Closing connection...")

        sock.close()

        print("\n\nElastic Container Service - ECS Client: Connection closed succesfully")

def removeContainer(containerName):

    sock = createConnection()

    print("\n\nElastic Container Service - ECS Client: Connection established on port 7070")

    try:

        print("\n\nElastic Container Service - ECS Client: Trying request...")

        clientRequest = ("remove " + containerName).encode()

        sock.sendall(clientRequest)

    finally:

        print("\n\nElastic Container Service - ECS Client: Request made succesfully")

        print("\n\nElastic Container Service - ECS Client: Closing connection...")

        sock.close()

        print("\n\nElastic Container Service - ECS Client: Connection closed succesfully")
