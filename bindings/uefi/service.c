//----------------------------------------------------------------------------
// Copyright Notice:
// Copyright 2017 Distributed Management Task Force, Inc. All rights reserved.
// License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libredfish/LICENSE.md
//----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>

#include <redfishService.h>
#include <redfishPayload.h>
#include <redpath.h>
#include <redfishEvent.h>

struct MemoryStruct
{
  char *memory;
  size_t size;
  char* origin;
  size_t originalSize;
};

struct EventCallbackRegister
{
    bool unregister;
    redfishEventCallback callback;
    unsigned int eventTypes;
    char* context;
    redfishService* service;
};

static int initCurl(redfishService* service);
static size_t curlWriteMemory(void *buffer, size_t size, size_t nmemb, void *userp);
static size_t curlReadMemory(void *buffer, size_t size, size_t nmemb, void *userp);
static int curlSeekMemory(void *userp, unsigned int offset, int origin);
static size_t curlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userp);
static redfishService* createServiceEnumeratorNoAuth(const char* host, const char* rootUri, bool enumerate, unsigned int flags);
static redfishService* createServiceEnumeratorBasicAuth(const char* host, const char* rootUri, const char* username, const char* password, unsigned int flags);
static redfishService* createServiceEnumeratorSessionAuth(const char* host, const char* rootUri, const char* username, const char* password, unsigned int flags);
static redfishService* createServiceEnumeratorToken(const char* host, const char* rootUri, const char* token, unsigned int flags);
static char* makeUrlForService(redfishService* service, const char* uri);
static json_t* getVersions(redfishService* service, const char* rootUri);
static char* getEventSubscriptionUri(redfishService* service);
static bool registerCallback(redfishService* service, redfishEventCallback callback, unsigned int eventTypes, const char* context);
static bool unregisterCallback(redfishEventCallback callback, unsigned int eventTypes, const char* context);
static void eventActorTask();
static void cleanupEventActor();
static void addStringToJsonObject(json_t* object, const char* key, const char* value);
static void addStringToJsonArray(json_t* array, const char* value);

redfishService* createServiceEnumerator(const char* host, const char* rootUri, enumeratorAuthentication* auth, unsigned int flags)
{
    if(auth == NULL)
    {
        return createServiceEnumeratorNoAuth(host, rootUri, true, flags);
    }
    if(auth->authType == REDFISH_AUTH_BASIC)
    {
        return createServiceEnumeratorBasicAuth(host, rootUri, auth->authCodes.userPass.username, auth->authCodes.userPass.password, flags);
    }
    else if(auth->authType == REDFISH_AUTH_BEARER_TOKEN)
    {
        return createServiceEnumeratorToken(host, rootUri, auth->authCodes.authToken.token, flags);
    }
    else if(auth->authType == REDFISH_AUTH_SESSION)
    {
        return createServiceEnumeratorSessionAuth(host, rootUri, auth->authCodes.userPass.username, auth->authCodes.userPass.password, flags);
    }
    else
    {
        return NULL;
    }
}

json_t* getUriFromService(redfishService* service, const char* uri)
{
    // UNSUPPORTED YET
    return NULL;
}

json_t* patchUriFromService(redfishService* service, const char* uri, const char* content)
{
    // UNSUPPORTED YET
    return NULL;
}

typedef struct {
    char* Location;
    char* XAuthToken;
} knownHeaders;

static size_t headerCallback(char* buffer, size_t size, size_t nitems, void* userdata)
{
    char* tmp;
    knownHeaders* headers = (knownHeaders*)userdata;
    if(headers == NULL)
    {
        return nitems * size;
    }
    if(strncasecmp(buffer, "Location:", 9) == 0)
    {
        headers->Location = strdup(buffer+10);
        tmp = strchr(headers->Location, '\r');
        if(tmp)
        {
            tmp[0] = 0;
        }
    }
    else if(strncasecmp(buffer, "X-Auth-Token:", 13) == 0)
    {
        headers->XAuthToken = strdup(buffer+14);
        tmp = strchr(headers->XAuthToken, '\r');
        if(tmp)
        {
            tmp[0] = 0;
        }
    }

    return nitems * size;
}


json_t* postUriFromService(redfishService* service, const char* uri, const char* content, size_t contentLength, const char* contentType)
{
    // UNSUPPORTED YET
    return NULL;
}

bool deleteUriFromService(redfishService* service, const char* uri)
{

    // UNSUPPORTED YET
    return false;
}

bool registerForEvents(redfishService* service, const char* postbackUri, unsigned int eventTypes, redfishEventCallback callback, const char* context)
{
    // UNSUPPORTED YET
    return false;
}

redfishPayload* getRedfishServiceRoot(redfishService* service, const char* version)
{
    json_t* value;
    json_t* versionNode;
    const char* verUrl;

    if(version == NULL)
    {
        versionNode = json_object_get(service->versions, "v1");
    }
    else
    {
        versionNode = json_object_get(service->versions, version);
    }
    if(versionNode == NULL)
    {
        return NULL;
    }
    verUrl = json_string_value(versionNode);
    if(verUrl == NULL)
    {
        return NULL;
    }
    value = getUriFromService(service, verUrl);
    if(value == NULL)
    {
        if((service->flags & REDFISH_FLAG_SERVICE_NO_VERSION_DOC) == 0)
        {
            json_decref(versionNode);
        }
        return NULL;
    }
    return createRedfishPayload(value, service);
}

redfishPayload* getPayloadByPath(redfishService* service, const char* path)
{
    redPathNode* redpath;
    redfishPayload* root;
    redfishPayload* ret;

    if(!service || !path)
    {
        return NULL;
    }

    redpath = parseRedPath(path);
    if(!redpath)
    {
        return NULL;
    }
    if(!redpath->isRoot)
    {
        cleanupRedPath(redpath);
        return NULL;
    }
    root = getRedfishServiceRoot(service, redpath->version);
    if(redpath->next == NULL)
    {
        cleanupRedPath(redpath);
        return root;
    }
    ret = getPayloadForPath(root, redpath->next);
    cleanupPayload(root);
    cleanupRedPath(redpath);
    return ret;
}

void cleanupServiceEnumerator(redfishService* service)
{
    // UNSUPPORTED YET
    return;
}


static int initCurl(redfishService* service)
{
     // UNSUPPORTED YET
    return 0;
}

static size_t curlWriteMemory(void *contents, size_t size, size_t nmemb, void *userp)
{
    // UNSUPPORTED YET
    return 0;
}

static size_t curlReadMemory(void *ptr, size_t size, size_t nmemb, void *userp)
{
    // UNSUPPORTED YET
    return 0;                          /* no more data left to deliver */
}

static int curlSeekMemory(void *userp, unsigned int offset, int origin)
{
    // UNSUPPORTED YET
    return 0;
}

static size_t curlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userp)
{
    char* header = NULL;
    char* tmp;

    if(userp == NULL)
    {
        return nitems * size;
    }
    if(strncmp(buffer, "Location: ", 10) == 0)
    {
        *((char**)userp) = strdup(buffer+10);
        header = *((char**)userp);
    }
    if(header)
    {
        tmp = strchr(header, '\r');
        if(tmp)
        {
            *tmp = 0;
        }
        tmp = strchr(header, '\n');
        if(tmp)
        {
            *tmp = 0;
        }
    }
    return nitems * size;
}

static redfishService* createServiceEnumeratorNoAuth(const char* host, const char* rootUri, bool enumerate, unsigned int flags)
{
    redfishService* ret;

    ret = (redfishService*)calloc(1, sizeof(redfishService));
    if(initCurl(ret) != 0)
    {
        free(ret);
        return NULL;
    }
    ret->host = strdup(host);
    ret->flags = flags;
    if(enumerate)
    {
        ret->versions = getVersions(ret, rootUri);
    }

    return ret;
}

static redfishService* createServiceEnumeratorBasicAuth(const char* host, const char* rootUri, const char* username, const char* password, unsigned int flags)
{
    redfishService* ret;

    ret = createServiceEnumeratorNoAuth(host, rootUri, false, flags);
    //curl_easy_setopt(ret->curl, CURLOPT_USERNAME, username);
    //curl_easy_setopt(ret->curl, CURLOPT_PASSWORD, password);
    ret->versions = getVersions(ret, rootUri);
    return ret;
}

static redfishService* createServiceEnumeratorSessionAuth(const char* host, const char* rootUri, const char* username, const char* password, unsigned int flags)
{
    redfishService* ret;
    redfishPayload* payload;
    redfishPayload* links;
    json_t* sessionPayload;
    json_t* session;
    json_t* odataId;
    const char* uri;
    json_t* post;
    char* content;

    ret = createServiceEnumeratorNoAuth(host, rootUri, true, flags);
    if(ret == NULL)
    {
        return NULL;
    }
    payload = getRedfishServiceRoot(ret, NULL);
    if(payload == NULL)
    {
        cleanupServiceEnumerator(ret);
        return NULL;
    }
    links = getPayloadByNodeName(payload, "Links");
    cleanupPayload(payload);
    if(links == NULL)
    {
        cleanupServiceEnumerator(ret);
        return NULL;
    }
    session = json_object_get(links->json, "Sessions");
    if(session == NULL)
    {
        cleanupPayload(links);
        cleanupServiceEnumerator(ret);
        return NULL;
    }
    odataId = json_object_get(session, "@odata.id");
    if(odataId == NULL)
    {
        cleanupPayload(links);
        cleanupServiceEnumerator(ret);
        return NULL;
    }
    uri = json_string_value(odataId);
    post = json_object();
    addStringToJsonObject(post, "UserName", username);
    addStringToJsonObject(post, "Password", password);
    content = json_dumps(post, 0);
    json_decref(post);
    sessionPayload = postUriFromService(ret, uri, content, 0, NULL);
    if(sessionPayload == NULL)
    {
        //Failed to create session!
        free(content);
        cleanupPayload(links);
        cleanupServiceEnumerator(ret);
        return NULL;
    }
    json_decref(sessionPayload);
    cleanupPayload(links);
    free(content);
    return ret;
}

static redfishService* createServiceEnumeratorToken(const char* host, const char* rootUri, const char* token, unsigned int flags)
{
    // UNSUPPORTED YET
    return NULL;
}

static char* makeUrlForService(redfishService* service, const char* uri)
{
    char* url;
    if(service->host == NULL)
    {
        return NULL;
    }
    url = (char*)malloc(strlen(service->host)+strlen(uri)+1);
    strcpy(url, service->host);
    strcat(url, uri);
    return url;
}

static json_t* getVersions(redfishService* service, const char* rootUri)
{
    if(service->flags & REDFISH_FLAG_SERVICE_NO_VERSION_DOC)
    {
        service->versions = json_object();
        if(service->versions == NULL)
        {
            return NULL;
        }
        addStringToJsonObject(service->versions, "v1", "/redfish/v1");
        return service->versions;
    }
    if(rootUri != NULL)
    {
        return getUriFromService(service, rootUri);
    }
    else
    {
        return getUriFromService(service, "/redfish");
    }
}

static char* getEventSubscriptionUri(redfishService* service)
{
    // UNSUPPORTED YET
    return NULL;
}

static bool registerCallback(redfishService* service, redfishEventCallback callback, unsigned int eventTypes, const char* context)
{
    // UNSUPPORTED YET
    return false;
}

static bool unregisterCallback(redfishEventCallback callback, unsigned int eventTypes, const char* context)
{
    // UNSUPPORTED YET
    return false;
}

static void freeRegistration(void* reg)
{
    // UNSUPPORTED YET
    return;
}

static int eventRegisterCallback()
{
    // UNSUPPORTED YET
    return 0;
}

static int eventReceivedCallback()
{
    // UNSUPPORTED YET
    return 0;
}

static void eventActorTask()
{
    // UNSUPPORTED YET
    return;
}

static void cleanupEventActor()
{
    // UNSUPPORTED YET
    return;
}

static void addStringToJsonObject(json_t* object, const char* key, const char* value)
{
    json_t* jValue = json_string(value);

    json_object_set(object, key, jValue);

    json_decref(jValue);
}

static void addStringToJsonArray(json_t* array, const char* value)
{
    json_t* jValue = json_string(value);

    json_array_append(array, jValue);

    json_decref(jValue);
}
