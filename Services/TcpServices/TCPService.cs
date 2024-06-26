﻿using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ExodusMudClient.Services.TcpServices
{
    public class TcpService : ITcpService, IDisposable
    {
        public Socket TcpSocket { get; private set; }
        private bool isConnected = false;
        private event Action<string> _dataReceived;
        private readonly object lockObject = new object();

        public TcpService()
        {
            TcpSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        }

        public event Action<string> DataReceived
        {
            add
            {
                lock (lockObject)
                {
                    _dataReceived += value;
                }
            }
            remove
            {
                lock (lockObject)
                {
                    _dataReceived -= value;
                }
            }
        }

        public void Connect(string ipAddress, int port)
        {
            if (isConnected)
            {
                Console.WriteLine("Already connected.");
                return;
            }

            try
            {
                IPAddress[] ipAddresses = Dns.GetHostAddresses(ipAddress);
                IPEndPoint endPoint = new IPEndPoint(ipAddresses[0], port);
                TcpSocket.Connect(endPoint);
                isConnected = true;
                Console.WriteLine("Connected!");
                Task.Run(() => ReadData());
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"SocketException in Connect: {ex.Message}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"General exception in Connect: {ex.Message}");
            }
        }

        public void Disconnect()
        {
            if (!isConnected) return;

            try
            {
                TcpSocket.Shutdown(SocketShutdown.Both);
                TcpSocket.Close();
                isConnected = false;
                Console.WriteLine("Disconnected.");
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"SocketException in Disconnect: {ex.Message}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"General exception in Disconnect: {ex.Message}");
            }
        }

        private StringBuilder dataBuffer = new StringBuilder();
        private StringBuilder sendBuff = new StringBuilder();

        //public void ReadData() {
        //    try {
        //        while (TcpSocket.Connected) {
        //            byte[] byteBuffer = new byte[1024];
        //            int received = TcpSocket.Receive(byteBuffer);
        //            if (received > 0) {
        //                string data = Encoding.UTF8.GetString(byteBuffer,0,received);
        //                dataBuffer.Append(data);
        //                ProcessBuffer();
        //                _dataReceived?.Invoke(data);
        //            }
        //        }
        //    } catch (SocketException ex) {
        //        Console.WriteLine($"SocketException in ReadData: {ex.Message}");
        //        Disconnect(); // Ensure clean disconnection on socket error
        //    } catch (Exception ex) {
        //        Console.WriteLine($"General exception in ReadData: {ex.Message}");
        //    }
        //}

        public void ReadData()
        {
            try
            {
                while (TcpSocket.Connected)
                {
                    byte[] byteBuffer = new byte[1024];
                    int received = TcpSocket.Receive(byteBuffer);
                    if (received > 0)
                    {
                        string data = Encoding.UTF8.GetString(byteBuffer, 0, received);
                        dataBuffer.Append(data); // Accumulate received data in a buffer
                        ProcessBuffer(); // Process the buffer for complete lines and handle partial data
                    }
                }
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"SocketException in ReadData: {ex.Message}");
                Disconnect(); // Ensure clean disconnection on socket error
            }
            catch (Exception ex)
            {
                Console.WriteLine($"General exception in ReadData: {ex.Message}");
            }
        }

        private void ProcessBuffer()
        {
            string bufferContent = dataBuffer.ToString();
            int lineEndIndex;
            // Loop through the buffer and process complete lines
            while ((lineEndIndex = bufferContent.IndexOf("\r")) != -1)
            {
                string line = bufferContent.Substring(0, lineEndIndex).Trim();
                _dataReceived?.Invoke(line);
                bufferContent = bufferContent.Substring(lineEndIndex + 1);
            }

            // Keep any partial line (incomplete data) in the buffer for next read
            dataBuffer.Clear();
            dataBuffer.Append(bufferContent);
        }

        public void SendData(string data)
        {
            if (!isConnected)
            {
                Console.WriteLine("Not connected. Cannot send data.");
                return;
            }

            try
            {
                byte[] buffer = Encoding.UTF8.GetBytes(data + "\r\n");
                TcpSocket.Send(buffer);
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"SocketException in SendData: {ex.Message}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"General exception in SendData: {ex.Message}");
            }
        }

        public void Dispose()
        {
            if (TcpSocket != null)
            {
                if (TcpSocket.Connected)
                {
                    Disconnect();
                }
                TcpSocket.Dispose();
            }
        }
    }
    public interface ITcpService
    {
        Socket TcpSocket { get; }
        event Action<string> DataReceived;
        void Connect(string ipAddress, int port);
        void ReadData();
        void SendData(string data);
    }
}
