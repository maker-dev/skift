#pragma once

struct Client;

struct AudioBuffer
{
private:
    int _id;
    Client *_client;
    int _samplerate;
    int _channels;

public:
    int id() { return _id; }
    Client *client() { return _client; }

    AudioBuffer(
        int id,
        struct Client *client,
        int samplerate,
        int channels);
};