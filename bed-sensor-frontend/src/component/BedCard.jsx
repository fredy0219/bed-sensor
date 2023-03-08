import React from 'react';
import { Card, CardActionArea, CardContent, Typography, Link } from '@mui/material';
import { WifiOffTwoTone, WarningTwoTone, HotelTwoTone, NotificationsActiveTwoTone, AirlineSeatReclineNormalTwoTone, DirectionsRunTwoTone, CancelPresentationTwoTone } from '@mui/icons-material';
import { Paper } from '@mui/material';
import { green, grey, yellow, red} from '@mui/material/colors';
import { styled } from '@mui/system';


export const checkColor = (state) => {
    var color = '#FFF'
    switch (state) {
        case 'Off':
            color = grey[500]; break;
        case 'No connection':
            color = red[200]; break;
        case 'No signal':
            color = red[200]; break;
        case 'Reposition':
            color = green[200]; break;
        default:
            color = grey[200];
    }
    return color
}

const StyledCard = styled(Card)(({state}) =>({
    backgroundColor: checkColor(state)
}));

const StyledCardDiv = styled('div')(({theme})=>({
    display: 'flex',
    justifyContent: 'center',
    marginBottom: theme.spacing(1),
}));

const StyledPaper = styled(Paper)(({theme}) => ({
    display: 'flex',
    justifyContent: 'center',
    marginBottom: theme.spacing(1),
    backgroundColor: yellow[300],
}));

const styledIcon = {
    fontSize: "150px",
};

const StyledCardTime = styled('div')(({state})=>({
    visibility: state == 'Reposition' ? 'visible' : 'hidden'
}));

export const BedIcon = (props) => {
    // const classes = useStyles(props);
    let icon = HotelTwoTone
    if (props.state == 'Off') icon = CancelPresentationTwoTone
    if (props.state == 'No connection') icon = WifiOffTwoTone
    if (props.state == 'No signal') icon = WarningTwoTone
    if (props.state == 'On bed') icon = HotelTwoTone
    if (props.state == 'Reposition') icon = NotificationsActiveTwoTone
    // if(props.state == 'Sit on bed') icon = SelfImprovementTwoTone
    if (props.state == 'Sit beside bed') icon = AirlineSeatReclineNormalTwoTone
    if (props.state == 'Escape') icon = DirectionsRunTwoTone
    return React.createElement(icon, { ...props });
}

const BedCard = (props) => {
    return (
        <div>
            {/* <Paper className={classes.cardDiv} style={{backgroundColor: yellow[300]}}> */}
            <StyledPaper>
                <Typography variant='h5'>
                    {props.info.number}
                </Typography>
            </StyledPaper>

            <StyledCard state={props.info.state}>
                <Link href={"/bedDetail/" + props.info.number} style={{textDecoration: 'inherit', }} color="inherit">
                    <CardActionArea>
                        <CardContent>
                            <StyledCardDiv>
                                <BedIcon sx={styledIcon} state={props.info.state} />
                            </StyledCardDiv>
                            <StyledCardDiv>
                                <Typography variant='h6'>
                                    {props.info.state}
                                </Typography>
                            </StyledCardDiv>
                            <StyledCardDiv>
                                <StyledCardTime variant='h6' state={props.info.state}>
                                    00:00:00
                                </StyledCardTime>
                            </StyledCardDiv>
                        </CardContent>
                    </CardActionArea>
                </Link>
            </StyledCard>
        </div>
    );
};

export default BedCard;
