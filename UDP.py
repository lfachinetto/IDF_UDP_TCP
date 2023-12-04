# Programa que recebe uma mensagem pela rede usando socket UDP
import socket
import math


def calculate(mensagem):
    try:
        c = int(mensagem.strip("\0"))
        envia = c + 1
    except :
        envia = "ERRO"
    return envia


# Endereco IP do receptor
HOST = "192.168.0.142"
# Porta que o receptor vai escutar
PORT = 8888  # informar aqui a porta que vai aguardar a mensagem
# O receptor sera um par de endereço e porta
orig = (HOST, PORT)
# Cria o socket
udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Conectando
udp.bind(orig)
print("Servidor UDP escutando na porta", PORT)
while True:
    # Recebe dados do socket, informações e endereço
    msg, cliente = udp.recvfrom(1024)
    mensagem = msg.decode()
    print("Recebeu: " + mensagem)
    envia = calculate(mensagem)
    print("Retornou: " + str(envia))
    udp.sendto(str(envia).encode(), cliente)
    # Fechamento do socekt
udp.close()
