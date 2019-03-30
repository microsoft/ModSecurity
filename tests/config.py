# Location of modsecurity.conf
modsecurity_conf_location = "C:\projects\ModSecurity\iis\wix\modsecurity.conf"

#
#[27/Sep/2018:15:35:09 --0700] - but python has issues with %z so we trim it
log_date_format = "%d/%b/%Y:%H:%M:%S"
#[27/Sep/2018:15:35:09 --0700]
log_date_regex = "\[(\d{1,2}\/[A-Z][a-z]{2}\/\d{4}:\d{1,2}:\d{1,2}:\d{1,2} )"
                 
# Regular expression to filter for timestamp in Apache Error Log
#
# Default timestamp format: (example: [Thu Nov 09 09:04:38.912314 2017])
# log_date_regex = "\[([A-Z][a-z]{2} [A-z][a-z]{2} \d{1,2} \d{1,2}\:\d{1,2}\:\d{1,2}\.\d+? \d{4})\]"
#
# Reverse format: (example: [2017-11-09 08:25:03.002312])
#log_date_regex = "\[([0-9-]{10} [0-9:.]{15})\]"

# Date format matching the timestamp format used by Apache 
# in order to generate matching timestamp ourself
#
# Default timestamp format: (example: see above)
# log_date_format = "%a %b %d %H:%M:%S.%f %Y"
#
# Reverse format: (example: see above)
#log_date_format = "%Y-%m-%d %H:%M:%S.%f"
failing_tests = ['920100-10', '920100-12', '920100-14', '920100-2', '920100-4', '920100-6', '920100-8', '920160-4', '920170-5', '920170-6', '920180-1', '920180-3', '920200-3', '920200-4', '920200-7', '920200-9', '920210-5', '920220-2', '920240-2', '920260-2', '920270-6', '920271-5', '920271-6', '920272-4', '920273-2', '920273-4', '920274-5', '920290-1', '920310-2', '920310-5', '920311-2', '920320-2', '920330-2', '920350-2', '920420-9', '920430-8', '920450-5', '920450-8', '920470-4', '920470-5', '932100-3', '933110-11', '933110-19', '933130-3', '933130-6', '933151-3', '933151-5', '933160-22', '933161-5', '941100-4', '941120-1', '941160-1FN', '941310-1', '942420-1', '943110-4', '944100-11', '944100-12', '944100-15', '944100-16', '944110-11', '944110-12', '944110-15', '944110-16', '944120-107', '944120-108', '944120-124', '944120-125', '944120-22', '944120-23', '944120-39', '944120-40', '944120-5', '944120-56', '944120-57', '944120-6', '944120-73', '944120-74', '944120-90', '944120-91', '944210-22', '944210-23', '944210-39', '944210-40', '944210-5', '944210-6', '920200-10', '920240-3', '920270-7', '920310-3', '920310-6', '920470-6', '942480-2', '920220-3', '941100-5FN', '941110-2', '933160-21', '920311-3', '921110-5', '942500-1']
