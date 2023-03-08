import { Box, Grid, Paper, Typography, Card, CardActionArea, CardContent, Button } from '@mui/material';
import { yellow } from '@mui/material/colors';
import { styled } from '@mui/system';
import React, { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';
import SensorPanel from './SensorPanel';
import { BedIcon, checkColor } from './BedCard';

import websocket from 'socket.io-client'

const StyledBedBox = styled(Box)({
    backgroundColor: '#EEEEEE',
    padding: 10,
    border: '1px dashed grey'
});

const StyledPaper = styled(Paper)(({ theme }) => ({
    display: 'flex',
    justifyContent: 'center',
    marginBottom: theme.spacing(1),
    backgroundColor: yellow[300],
    fontSize: "30px",
}));

const StyledCard = styled(Card)(({ state }) => ({
    backgroundColor: checkColor(state)
}));

const StyledCardDiv = styled('div')(({ theme }) => ({
    display: 'flex',
    justifyContent: 'center',
    marginBottom: theme.spacing(1),
}));

const styledIcon = {
    fontSize: "150px",
}

const StyledCardTime = styled('div')(({ state }) => ({
    visibility: state == 'Reposition' ? 'visible' : 'hidden'
}));

const BedDetail = (props) => {
    const [bedInfo, setBedInfo] = useState({number:0, state:'Reposition', time:1234})
    const [bedSwitch, setBedSwitch] = useState('OFF')
    const [ws, setWs] = useState(null)
    const params = useParams()
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
                // ws.emit('askBedOverview', 'Hello World')
                var message = {
                    bedId: params.bedId,
                }
                ws.emit('askBedDetail', message)
            }
        }, 100);

        
    }, [ws])

    // websocket handler
    const initWebSocket = () => {
        ws.on('getBedDetail', message => {
            // console.log(message['data'])
            changePanelColor(message['data'])
        })
    }

    function changePanelColor(colorArr){
    
        for(let i = 0 ; i < 80 ; i++){
          const index = (9-Math.floor(i/8)) + ((i%8)*10)
          const coloum = i % 8 + 1;
          const row = Math.floor(i/8) + 1;
          var dot = document.getElementById(1+','+coloum+','+row);
          var c = colorArr[index]
          c = c > 1.0? 1.0 : c;
          dot.style.backgroundColor = 'hsl('+1+','+100+'%,'+(100-80*c)+'%)';
        //   dot.textContent = Math.round(c*100)/100;
        }
    
        for(let i = 0 ; i < 80 ; i++){
          const index = (9-Math.floor(i/8)) + ((i%8)*10)
          const coloum = i % 8 + 1;
          const row = Math.floor(i/8) + 1;
          dot = document.getElementById(2+','+coloum+','+row);
          c = colorArr[index+80]
          dot.style.backgroundColor = 'hsl('+1+','+100+'%,'+(100-80*c)+'%)';
        //   dot.textContent = Math.round(c*100)/100;
        }
    
        for(let i = 0 ; i < 80 ; i++){  
          const index = (9-Math.floor(i/8)) + ((i%8)*10)
          const coloum = i % 8 + 1;
          const row = Math.floor(i/8) + 1;
          dot = document.getElementById(3+','+coloum+','+row);
          c = colorArr[index+160]
          dot.style.backgroundColor = 'hsl('+1+','+100+'%,'+(100-80*c)+'%)';
        //   dot.textContent = Math.round(c*100)/100;
        }
        
      }

    function connectBedSensor(){
        var tmpBedSwitch = bedSwitch

        if(tmpBedSwitch == 'OFF'){
            tmpBedSwitch = 'ON'
            setBedSwitch('ON')
        }else{
            tmpBedSwitch = 'OFF'
            setBedSwitch('OFF')
        }

        var message = {
            bedId: params.bedId,
            switch: tmpBedSwitch
        }
        
        ws.emit('connectBedSensor', message)
    }

    return (

        <Grid container spacing={2} columns={5}>
            <Grid item xs={4}>
                <StyledPaper>
                    <Typography variant='h5'>
                        {params.bedId}
                    </Typography>
                </StyledPaper>
                <StyledBedBox>
                    <Grid container spacing={2} columns={3}>
                        <Grid item xs={1}>
                            <SensorPanel panel_index='1' />
                        </Grid>
                        <Grid item xs={1}>
                            <SensorPanel panel_index='2' />
                        </Grid>
                        <Grid item xs={1}>
                            <SensorPanel panel_index='3' />
                        </Grid>
                    </Grid>
                </StyledBedBox>
            </Grid>
            <Grid item xs={1}>
                <Grid container spacing={1}>
                    <Grid item xs={12}>
                        <StyledCard state={bedInfo.state}>
                            <CardActionArea>
                                <CardContent>
                                    <StyledCardDiv>
                                        <BedIcon sx={styledIcon} state={bedInfo.state} />
                                    </StyledCardDiv>
                                    <StyledCardDiv>
                                        <Typography variant='h6'>
                                            {bedInfo.state}
                                        </Typography>
                                    </StyledCardDiv>
                                    <StyledCardDiv>
                                        <StyledCardTime variant='h6' state={bedInfo.state}>
                                            {bedInfo.time}
                                        </StyledCardTime>
                                    </StyledCardDiv>
                                </CardContent>
                            </CardActionArea>
                        </StyledCard>
                    </Grid>
                    <Grid item xs={12}><Button fullWidth variant="contained">Mute</Button></Grid>
                    <Grid item xs={12}><Button fullWidth variant="contained">Reset</Button></Grid>
                    <Grid item xs={12}><Button fullWidth variant="contained" onClick={connectBedSensor}>{bedSwitch}</Button></Grid>
                </Grid>
            </Grid>
        </Grid >

    );
};

export default BedDetail;