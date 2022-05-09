import socket

def createConnection():

    return socket.create_connection(("localhost", 7070))

def createContainer(containerName):

    sock = createConnection()

    print("Connection established on port 7070")

    try:

        print("Trying request...")

        clientRequest = ("create " + containerName).encode()

        sock.sendall(clientRequest)

    finally:

        print("Request made succesfully")

        print("Closing connection...")

        sock.close()

        print("Connection closed succesfully")

def listContainers():

    sock = createConnection()

    print("Connection established on port 7070")

    try:

        print("Trying request...")

        clientRequest = ("list").encode()

        sock.sendall(clientRequest)

    finally:

        print("Request made succesfully")

        print("Closing connection...")

        print("Connection closed succesfully")

        sock.close()

def stopContainer(containerName):

    sock = createConnection()

    print("Connection established on port 7070")

    try:

        print("Trying request...")

        clientRequest = ("stop " + containerName).encode()

        sock.sendall(clientRequest)

    finally:

        print("Request made succesfully")

        print("Closing connection...")

        sock.close()

        print("Connection closed succesfully")

def removeContainer(containerName):

    sock = createConnection()

    print("Connection established on port 7070")

    try:

        print("Trying request...")

        clientRequest = ("remove " + containerName).encode()

        sock.sendall(clientRequest)

    finally:

        print("Request made succesfully")

        print("Closing connection...")

        sock.close()

        print("Connection closed succesfully")
