import { AppBar, Toolbar, Typography } from '@mui/material';
// import { makeStyles } from '@mui/styles';
// const useStyles = makeStyles((theme) => ({
// }));

const NavBar = () => {
    // const classes = useStyles();
    return (
        <AppBar position="fixed">
            <Toolbar>
                <Typography variant='h6'>
                    Aeolus Bed Sensor
                </Typography>
            </Toolbar>
        </AppBar>
    );
};

export default NavBar;
