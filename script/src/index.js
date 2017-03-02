import ruter from 'ruter-api'
import moment from 'moment'
import { forEach, chunk, range } from 'lodash'
import { createWriteStream } from 'fs'

function outputLines(lines) {
    if (process.argv.length > 2) {
        const file = createWriteStream(process.argv[2])

        file.on('error', (err) => {
            console.log(err)
        })

        file.write(lines)

        file.end()
    } else {
        console.log(lines)
    }
}

function chunksToString(title, chunks) {
    const result = []

    if (chunks.length > 0) {
        result.push(title)
        result.push(...chunks[0])
    }

    if (chunks.length > 1) {
        result.push(`${title} 2`)
        result.push(...chunks[1])
    }

    return result.join('\n')
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

        const display = `${line} ${route.substring(0, 6)}-${ts}`

        if (platform === '1') {
            mot.push(display)
        } else {
            fra.push(display)
        }
    })

    let lines = [chunksToString("Mot sentrum", chunk(mot, 3)), chunksToString("Fra sentrum", chunk(fra, 3))].join('\n')

    forEach(range(16 - lines.split('\n').length), () => {
        lines = `${lines}\n`
    })

    outputLines(lines)
})
