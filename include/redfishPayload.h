//----------------------------------------------------------------------------
// Copyright Notice:
// Copyright 2017 Distributed Management Task Force, Inc. All rights reserved.
// License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libredfish/LICENSE.md
//----------------------------------------------------------------------------
#ifndef _REDFISH_PAYLOAD_H_
#define _REDFISH_PAYLOAD_H_

//redfishPayload is defined here...
#include "redfishService.h"

#include "redpath.h"

REDFISH_EXPORT redfishPayload* createRedfishPayload(json_t* value, redfishService* service);
REDFISH_EXPORT redfishPayload* createRedfishPayloadFromString(const char* value, redfishService* service);
REDFISH_EXPORT bool            isPayloadCollection(redfishPayload* payload);

REDFISH_EXPORT char*           getPayloadStringValue(redfishPayload* payload);
REDFISH_EXPORT int             getPayloadIntValue(redfishPayload* payload);

REDFISH_EXPORT redfishPayload* getPayloadByNodeName(redfishPayload* payload, const char* nodeName);
REDFISH_EXPORT redfishPayload* getPayloadByIndex(redfishPayload* payload, size_t index);
REDFISH_EXPORT redfishPayload* getPayloadForPath(redfishPayload* payload, redPathNode* redpath);
REDFISH_EXPORT redfishPayload* getPayloadForPathString(redfishPayload* payload, const char* string);
REDFISH_EXPORT size_t          getCollectionSize(redfishPayload* payload);
REDFISH_EXPORT redfishPayload* patchPayloadStringProperty(redfishPayload* payload, const char* propertyName, const char* value);
REDFISH_EXPORT redfishPayload* postContentToPayload(redfishPayload* target, const char* data, size_t dataSize, const char* contentType);
REDFISH_EXPORT redfishPayload* postPayload(redfishPayload* target, redfishPayload* payload);
REDFISH_EXPORT bool            deletePayload(redfishPayload* payload);
REDFISH_EXPORT char*           payloadToString(redfishPayload* payload, bool prettyPrint);
REDFISH_EXPORT void            cleanupPayload(redfishPayload* payload);

#endif
