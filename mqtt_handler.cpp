#include "mqtt_handler.h"
#include <algorithm>

#if defined(MQTT_DEBUG) && defined(MQTT_DEBUG_PORT)
#define DBG  MQTT_DEBUG_PORT.print
#define DBGL MQTT_DEBUG_PORT.println
#define DBGF MQTT_DEBUG_PORT.printf
#else
#define DBG(...)
#define DBGL(...)
#define DBGF(...)
#endif

int get_retry_delay(int retry_count, int retry_interval)
{
        if(retry_count < 5)
        {
            return retry_interval;
        } 
        else
        {
            return 2*retry_count*retry_interval;
        }
}

void mqtt_handler::mqtt_callback(char* topic, uint8_t* data, unsigned int length)
{
    String the_topic(topic);

    auto result=std::find_if(handlers.begin(),handlers.end(),[&](const handler_data& hd){return hd.topic==the_topic;});
    if(result!=handlers.end())
    {
        (*result).handler(topic,data,length);
    }
}

void mqtt_handler::check_subscriptions()
{
    for(auto& hd:handlers)
    if(!hd.subscribed)
    {
        hd.subscribed=client.subscribe(hd.topic.c_str());
    }
    last_subscription_check=millis();
}

mqtt_handler::mqtt_handler(): client(wifi_client), connecting(false), connected(false), connection_retry_interval(1000), connection_retry_count(0), subscription_check_interval(2500)
{
}

void mqtt_handler::begin(IPAddress ip, const String& client_id, const String& user, const String& pass)
{
    client.setServer(ip, 1883);
    client.setCallback([this](char* t, uint8_t* d, unsigned int l){mqtt_callback(t,d,l);});
    mqtt_id=client_id;
    mqtt_user=user;
    mqtt_pass=pass;
    can_connect=true;
}

void mqtt_handler::update()
{
    unsigned long now=millis();
    if(!client.connected())
    {
        DBGL(F("Not connected"));
        if(!can_connect)
            return;

        if(connected)
        {
            connected=false;

            for(auto& hd:handlers)
                hd.subscribed=false;

            if(on_disconnected)
                on_disconnected();
            DBGL(F("Marked as disconnected"));
        }
        if(!connecting)
        {
            DBGL("Starting to connect");
            connecting=true;
            connecting_start_time=now;
            client.connect(mqtt_id.c_str(), mqtt_user.c_str(), mqtt_pass.c_str());
            DBGL(F("Connection request sent"));
        }
        else if(now-connecting_start_time>(unsigned)get_retry_delay(connection_retry_count, connection_retry_interval))
        {
            connection_retry_count++;
            // retry
            DBGL(F("Retry"));
            connecting=false;
        }
    }
    else
    {
        DBGL(F("Connected"));
        if(connecting)
        {
            DBGL(F("Done connecting"));
            connecting=false;
            connected=true;

            check_subscriptions();

            if(on_connected)
                on_connected();
        }
        if(now-last_subscription_check>subscription_check_interval)
        {
            DBGL(F("Checking subscriptions"));
            check_subscriptions();
        }
        client.loop();
    }
}
void mqtt_handler::handle_topic(const String& name, topic_handler handler)
{
    handlers.push_back({name,handler,false});
    if(client.connected())
    {
        client.subscribe(name.c_str());
        handlers.back().subscribed=true;
    }
}

void mqtt_handler::publish(const String& topic, const String& msg)
{
    if(!connected)
        return;

    client.publish(topic.c_str(),msg.c_str());
}

mqtt_handler MQTT;
