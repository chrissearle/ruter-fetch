import ruter from 'ruter-api'
import moment from 'moment'
import { forEach, chunk, range, flatten, slice } from 'lodash'
import { createWriteStream } from 'fs'

function padToLength(data, length) {
    forEach(range(length - data.length), () => {
        data.push('')
    })

    return data
}

function outputLines(lines) {
    const output = slice(lines, 0, 16).join('\n')

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

function createSingleBlock(title, lines) {
    return padToLength(flatten([
        title,
        lines
    ]), 4)
}

function chunksToString(title, chunks) {
    const result = []

    if (chunks.length > 0) {
        result.push(createSingleBlock(title, chunks[0]))
    }

    if (chunks.length > 1) {
        result.push(createSingleBlock(`${title} 2`, chunks[1]))
    }

    return flatten(result)
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

    const motLines = chunksToString('Mot sentrum', chunk(mot, 3))
    const fraLines = chunksToString('Fra sentrum', chunk(fra, 3))

    const lines = flatten([motLines, fraLines])

    if (lines.length > 8) {
        outputLines(padToLength(lines, 16))
    } else {
        outputLines(padToLength(flatten([lines, lines], 16)))
    }
})
