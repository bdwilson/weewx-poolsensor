# 
# PoolService module to read temp from webservice
# 5/2020 - bubba@bubba.org
#
# Required settings to configure in weewx.conf:
#    webservice_url : URL of webservice delivering data
#    offset : set to 0 if not needed; otherwise postive or negative
#             temp offsets
#    [value from webserver] : [field to map to in Weewx]
#            In my example below, webserver returns {"Temperature":"75"}
#            and 75 would get saved to weewx extraTemp1
#  
#    You will also need to add another data_service entry in weewx.conf  
#           data_services = user.pool.PoolService  
#
# [PoolService]
#    webservice_url = http://192.168.1.114/temp
#    offset = 0.1                
#    Temperature = extraTemp1
#

import syslog
import weewx
from weewx.wxengine import StdService
import urllib2
import json

class PoolService(StdService):
    def __init__(self, engine, config_dict):
        super(PoolService, self).__init__(engine, config_dict)
        d = config_dict.get('PoolService', {})
	self.webservice_url = d.get('webservice_url')
        self.offset = float(d.get('offset'))
        self.items = {}
        for i in d:
            if (i != "webservice_url" and i != "offset"):
                self.items[i]=d[i]
        if (self.webservice_url != "None" and self.items):
            syslog.syslog(syslog.LOG_INFO, "PoolService: using URL %s" % self.webservice_url)
            self.bind(weewx.NEW_ARCHIVE_RECORD, self.read_json)

    def read_json(self, event):
	data= {}
        try:
            response = urllib2.urlopen(self.webservice_url, timeout=10)
            data = json.load(response)
            for key in data:
                if (key in self.items):
                    val = data[key]
                    map = self.items[key]
                    if (self.offset is not None):
                        val = float(val) + float(self.offset)
            	        syslog.syslog(syslog.LOG_INFO, "PoolService: found entry %s -> %s with value of %s using offset of %s to get %s " % (str(key), str(map), str(data[key]), str(self.offset), str(val)))
                    else:
            	        syslog.syslog(syslog.LOG_INFO, "PoolService: found entry %s -> %s with value of %s" % (str(key), str(map), str(data[key])))
            	    event.record[map] = float(val)
        except Exception as e:
            syslog.syslog(syslog.LOG_ERR, "PoolService: cannot read value: %s" % e)
