import React, { useState, useEffect } from 'react';
import { Grid } from '@mui/material';
import BedCard from './BedCard';
import websocket from 'socket.io-client'

const BedOverview = () => {
    const [ws, setWs] = useState(null)
    useEffect(() => {
        if (!ws) {
            console.log('no connection')
            setWs(websocket('http://localhost:5000'))
        }

        if (ws) {
            initWebSocket()
        }

        setInterval(() => {
            if (ws !== null) {
                ws.emit('askBedOverview', 'Hello World')
                ws.emit('askAlarmDataFromClient', 'Hello World')
            }
        }, 1000);
    }, [ws])

    // websocket handler
    const initWebSocket = () => {
        ws.on('getBedOverview', message => {
            setBedInfo(message)
        })
    }

    const [bedInfo, setBedInfo] = useState([
        { number: 168, state: 'No signal' },
        { number: 169, state: 'On bed' },
        { number: 170, state: 'On bed' },
        { number: 171, state: 'Reposition' },
        { number: 172, state: 'Off' }
    ]);

    return (
        <Grid container spacing={3}>
            {Array.from(bedInfo).map((info, index) => (
                <Grid item xs={3} key={index}>
                    <BedCard info={info} />
                </Grid>
            ))}
        </Grid>
    );
};

export default BedOverview;