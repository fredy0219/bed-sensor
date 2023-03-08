import React, { Component }  from 'react';
import { Container } from '@mui/icons-material';
import { makeStyles } from '@mui/styles';
import { useTheme } from '@mui/styles';
const useStyles = makeStyles((theme) => ({
  container: {
    paddingTop: useTheme.specing(10),
  }
}));

const RightBar = () => {
  const classes = useStyles();
  return (
    <Container className={classes.container}>
      Right
    </Container>
  );
};

export default RightBar;
