import ruter from 'ruter-api'
import moment from 'moment'
import { createWriteStream } from 'fs'

function outputLines(data) {
    const output = JSON.stringify(data)

    if (process.argv.length > 2) {
        const file = createWriteStream(process.argv[2])

        file.on('error', (err) => {
            console.log(err)
        })

        file.write(output)

        file.end()
    } else {
        console.log(output)
    }
}

ruter.api('StopVisit/GetDepartures/3010360', {}, response => {
    const json = response['result']

    const mot = []
    const fra = []

    json.map((item) => {
        const mvj = item['MonitoredVehicleJourney']
        const mc = mvj['MonitoredCall']

        const line = mvj['PublishedLineName']
        const platform = mvj['DirectionName']
        const timestamp = mc['ExpectedDepartureTime']
        const route = mc['DestinationDisplay'].split(' ')[0]

        const ts = moment(timestamp).format('HH:mm')

        const slot = {
            line: line,
            dest: route.substring(0, 15),
            time: ts
        }

        if (platform === '1') {
            mot.push(slot)
        } else {
            fra.push(slot)
        }
    })

    outputLines({
        up: {
            title: 'Mot sentrum',
            times: mot.slice(0, 6)
        },
        down: {
            title: 'Fra sentrum',
            times: fra.slice(0, 6)
        }
    })
})
