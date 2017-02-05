import fetch from 'node-fetch'
import moment from 'moment'

fetch('http://reisapi.ruter.no/StopVisit/GetDepartures/3010360')
    .then(function(response) {
        return response.json()
    })
    .then(function(json) {
        let maxCount = 6

        if (json.length < maxCount) {
            maxCount = json.length
        }

        const mot = []
        const fra = []

        json.slice(0, maxCount).map((item) => {
            const mvj = item['MonitoredVehicleJourney']
            const mc = mvj['MonitoredCall']

            const line = mvj['PublishedLineName']
            const platform = mvj['DirectionName']
            const timestamp = mc['ExpectedDepartureTime']
            const route = mc['DestinationDisplay'].split(' ')[0]

            const ts = moment(timestamp).format('HH:mm')

            const display = `${line} ${route.substring(0, 6)}-${ts}`

            if (platform === '1') {
                mot.push(display)
            } else {
                fra.push(display)
            }
        })

        console.log('Mot sentrum')
        console.log(mot.join('\n'))

        console.log('Fra sentrum')
        console.log(fra.join('\n'))
    })
