import { createTheme } from "@mui/system";
import { blue } from "@mui/material/colors";

export const theme = createTheme({

    bedOverview:{

    },
    bedCard:{
        card: {
        },
        cardDiv: {
            display: 'flex',
            justifyContent: 'center',
            marginBottom: 1
            
        },
        cardRoomPaper:{
            display: 'flex',
            justifyContent: 'center',
            marginBottom: 1
        },
        cardRoom: {
            display: 'flex',
            justifyContent: 'center',
            marginBottom: 1
            
        
        },
        cardIcon: {
            fontSize: "150px",
        },
        cardInfo: {
        },
        cardTime: {
            visibility: (props) => props.state == 'Reposition' ? 'visible' : 'hidden'
        }
    },
    palette: {
        primary:{
            main: blue[50]
        },
    },
    myButton:{
        backgroundColor: "red",
        color: "white",
        border: "1px solid black",
    }
});