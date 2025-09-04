#!/bin/bash
if [ $# != 2 ]
then
    echo "Usage: ./flash.sh filename BMCIP"
    exit 0
fi
FILENAME=$1
BMCIP=$2

if [ "${BMCIP}" = "" ] || [ "${FILENAME}" = "" ] || [ ! -f ${FILENAME} ]
then
    echo "Usage: ./flash.sh filename BMCIP"
    exit 1
fi


# --- minimal addition: password from env or prompt ---
if [ -z "$BMC_PASS" ]; then
  read -s -p "Enter BMC password for root: " BMC_PASS
  echo
fi
# -----------------------------------------------------

flashBIOS() {
    echo "Will Flash ${BMCIP}, tar file is ${FILENAME}"
    local bmc=${BMCIP}
    echo "${bmc} create session"
    #local token=`curl -s -k -H "Content-Type: application/json" -X POST https://${bmc}/login -d '{"username" :  "root", "password" :  "Loongson@2024"}' | grep token | awk '{print $2;}' | tr -d '"'`
    local token=`curl -s -k -H "Content-Type: application/json" -X POST https://${bmc}/login -d "{\"username\":\"root\",\"password\":\"${BMC_PASS}\"}" | grep token | awk '{print $2;}' | tr -d '"'`
    ret=$?
    if [ $ret -ne 0 ] || [ ! -n "$token" ]
    then
        echo "${bmc} create session error"
        return 2
    fi
    echo "${bmc} upload image"
    #curl -s -k -H "X-Auth-Token: $token" -H "Content-Type:application/x-tar"  -X POST --data-binary @"${FILENAME}" https://${bmc}/redfish/v1/UpdateService > /dev/null
    rep=`curl -s -k -H "X-Auth-Token: $token" -H "Content-Type:application/x-tar"  -X POST --data-binary @"${FILENAME}" https://${bmc}/redfish/v1/UpdateService`
    ret=$?
    if [ $ret -ne 0 ]
    then
        echo "${bmc} upload image error"
        return 6
    fi
    echo "${bmc} start update image"
    local taskUri=`echo $rep | grep Id | awk '{print $7;}' | tr -d '"' | tr -d ','`
    #local taskUri=`curl -s -k -H "X-Auth-Token: $token" -H "Content-Type:application/json"  https://${bmc}/redfish/v1/UpdateService | grep "fwUpdateTaskId" | awk '{print $2;}' | tr -d '"'`
    ret=$?
    if [ $ret -ne 0 ] || [ ! -n "$taskUri" ]
    then
        echo "${bmc} start update image error ,ret is $$ret , task Id is $taskUri "
        return 7
    fi
    taskUri=/redfish/v1/TaskService/Tasks/$taskUri
    sleep 10
    local flashpercent=`curl -s -k -H "X-Auth-Token: $token" -H "Content-Type: application/json" https://${bmc}$taskUri | grep "PercentComplete" | awk '{print $2;}' | tr -d ','`
    ret=$?
    if [ $ret -ne 0 ] || [ ! -n "$flashpercent" ]
    then
        echo "Get ${bmc} Task Failed : $taskUri, Please wait 3 minutes to retry"
        return 8
    fi
    local TaskStatus=`curl -s -k -H "X-Auth-Token: $token" -H "Content-Type: application/json" https://${bmc}$taskUri | grep "TaskState" | awk '{print $2;}' | tr -d ',' | tr -d '"'`
    ret=$?
    if [ $ret -ne 0 ] || [ $TaskStatus == "Exception" ]
    then
        echo "Get ${bmc} Task Status Exception : $taskUri, Please wait 3 minutes to retry"
        echo "TaskStatus is $TaskStatus"
        return 8
    fi
    local cnt=0
    while [ ${flashpercent} -lt 100 ]
    do
        flashpercent=`curl -s -k -H "X-Auth-Token: $token" -H "Content-Type: application/json" https://${bmc}$taskUri | grep "PercentComplete" | awk '{print $2;}' | tr -d ','`
        ret=$?
        if [ $ret -ne 0 ] || [ ! -n "$flashpercent" ]
        then
            echo "Get ${bmc} Task Failed : $taskUri, Please wait 3 minutes to retry"
            return 9
        fi
        TaskStatus=`curl -s -k -H "X-Auth-Token: $token" -H "Content-Type: application/json" https://${bmc}$taskUri | grep "TaskState" | awk '{print $2;}' | tr -d ',' | tr -d '"'` 
        ret=$?
        if [ $ret -ne 0 ] || [ $TaskStatus == "Exception" ]
        then
            echo "${bmc} Task Exception : $taskUri, Please wait 3 minutes to retry"
            echo "TaskStatus is $TaskStatus"
            return 9
        fi
        echo "${bmc} flash ${flashpercent}%"
        sleep 2
        cnt=$(( $cnt + 1 ))
        if [ $cnt -gt 180 ]
          then
            echo "flash ${bmc} over 6 minutes, so timeout"
            return 10
        fi
    done

    if [ ${flashpercent} -ne 100 ]
    then
      return 11
    fi
    echo "flash ${bmc} bios complete!"
    return 0
}
flashBIOS
ret=$?
exit $ret

