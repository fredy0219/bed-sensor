import { Container, Typography, Link } from '@mui/material';
import { CalendarToday, Settings, Report, Help, Home } from '@mui/icons-material'
import { styled } from '@mui/system';
import { blue ,grey} from '@mui/material/colors';

const StyledContainer = styled(Container)(({theme}) =>({
    height: "100vh",
    color: "white",
    paddingTop: theme.spacing(10),
    backgroundColor: blue[50],
    [theme.breakpoints.up("sm")]: {
        backgroundColor: grey[200],
        color: "#555",
        border: "1px solid #ece7e7",
    }

}));

const StyledDiv = styled('div')(({theme}) =>({
    display: "flex",
    alignItems: "center",
    marginBottom: theme.spacing(3),
}));

const iconStyle = {
    marginRight: 1
};

const textStyle = {
    fontWeight: 500,
};

const LeftBar = () => {
    return (
        <StyledContainer>
            <StyledDiv>
                <Home sx={iconStyle} />
                <Typography 
                sx={textStyle}
                component={Link}
                href="/bedOverview"
                style={{textDecoration: 'inherit', }}
                color="inherit"
                >Homepage</Typography>
            </StyledDiv>
            <StyledDiv>
                <CalendarToday sx={iconStyle} />
                <Typography sx={textStyle}>Calendar</Typography>
            </StyledDiv>

            <StyledDiv>
                <Settings sx={iconStyle} />
                <Typography sx={textStyle}>Setting</Typography>
            </StyledDiv>

            <StyledDiv>
                <Report sx={iconStyle} />
                <Typography sx={textStyle}>Report</Typography>
            </StyledDiv>

            <StyledDiv>
                <Help sx={iconStyle} />
                <Typography sx={textStyle}>Help</Typography>
            </StyledDiv>
        </StyledContainer>
    );
};

export default LeftBar;
