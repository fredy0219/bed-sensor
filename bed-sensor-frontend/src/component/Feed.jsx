import { Container, makeStyles } from '@material-ui/core';
import BedCard from './BedCard';
import BedOverview from './BedOverview';
import BedDetail from './BedDetail';


const useStyles = makeStyles((theme) => ({
  container: {
    paddingTop: theme.spacing(10),
  }
}));

const Feed = (props) => {
  const classes = useStyles();

  let contentPage = BedOverview
  if(props.children == '/') contentPage = BedOverview;
  if(props.children == '/bed_sensor') contentPage = BedDetail;
  
  return (
    <Container className={classes.container}>
      <BedOverview/>
    </Container>
  );
};

export default Feed;
