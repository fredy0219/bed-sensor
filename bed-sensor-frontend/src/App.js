import React from 'react';
import './App.css';
import { Container, Grid } from '@mui/material';
import NavBar from './component/NavBar';
import LeftBar from './component/LeftBar';
import BedOverview from './component/BedOverview';
import BedDetail from './component/BedDetail';
import { Route } from 'react-router';
import { BrowserRouter, Routes } from 'react-router-dom';

const containerStyle = {
  paddingTop: 10
}

function App(props) {
  return (
    <div>
        <BrowserRouter>
          <NavBar />
          <Grid container specing={1}>
            <Grid item sm={2} xs={2}>
              <LeftBar />
            </Grid>
            <Grid item sm={10} xs={10}>
              <Container sx={containerStyle}>
                <Routes>
                  <Route exact path="/bedOverview" element={<BedOverview />} />
                  <Route path="/bedDetail/:bedId" element={<BedDetail />} />
                </Routes>
              </Container>
            </Grid>
            {/* <Grid item sm={3}>
              <RightBar/>
            </Grid> */}
          </Grid>
        </BrowserRouter>
    </div>
  );
}

// ReactDOM.render(
//   (
//     <Router>
//       <Route path="/" component={App}>
//         <Route path="bed_sensor" component={Feed}/>
//       </Route>
//     </Router>
//   ),
//   document.getElementById('root')
// );


export default App;
