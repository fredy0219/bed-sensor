import React, { useState } from 'react'
import { Paper, Grid, Card } from '@mui/material';
// import { makeStyles } from '@mui/styles';
// import { useTheme } from '@mui/styles';
import { styled } from '@mui/system';

const StyledPaper = styled(Paper)(({theme}) => ({
    padding: theme.spacing(0.1),
    textAlign: 'center',
    elevation: 3,
    height: 15,
    border: "5px solid",
    borderColor: 'rgb(180,0,0,0)'
}));

const Dot = (props) => {
    return (
        <StyledPaper
            id={props.panel_index + ',' + props.x_index + ',' + props.y_index} />
    )
}

const SensorPanel = (props) => {
    return (
        <Grid container spacing={1} columns={16}>
            {Array.from(Array(10)).map((_, y) => {
                return Array.from(Array(8)).map((_, x) => (
                    <Grid item xs={2}>
                        <Dot panel_index={props.panel_index} x_index={x + 1} y_index={y + 1}></Dot>
                    </Grid>
                ))
            })}
        </Grid>
    );
};

export default SensorPanel;