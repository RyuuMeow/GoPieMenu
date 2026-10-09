// Original procedural score and UI sounds; no samples or third-party recordings.
// Rebuild with npm run audio. Uses only Node's standard library.
import { mkdirSync, writeFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
const out = fileURLToPath(new URL("../public/audio/", import.meta.url));
mkdirSync(out, {recursive:true});
const sr=44100, tau=2*Math.PI;
let seed=713;
const noise=()=>{seed=(1664525*seed+1013904223)>>>0;return seed/2147483648-1;};
const hz=n=>440*2**((n-69)/12);
const track=seconds=>[new Float32Array(Math.ceil(seconds*sr)),new Float32Array(Math.ceil(seconds*sr))];
function add(buf,start,duration,fn,gain=1,pan=0){
  const offset=Math.round(start*sr), length=Math.floor(duration*sr);
  const l=Math.sqrt((1-pan)/2),r=Math.sqrt((1+pan)/2);
  for(let i=0;i<length && offset+i<buf[0].length;i++){
    const t=i/sr;const sample=fn(t,i)*gain*Math.min(1,t/.003,(duration-t)/.015);
    buf[0][offset+i]+=sample*l;buf[1][offset+i]+=sample*r;
  }
}
function save(name,buf,peak=.65){
  let max=0,squares=0;
  for(let i=0;i<buf[0].length;i++)for(let c=0;c<2;c++){max=Math.max(max,Math.abs(buf[c][i]));squares+=buf[c][i]**2;}
  const gain=peak/Math.max(max,.001),data=Buffer.alloc(44+buf[0].length*4);
  data.write("RIFF",0);data.writeUInt32LE(data.length-8,4);data.write("WAVEfmt ",8);data.writeUInt32LE(16,16);data.writeUInt16LE(1,20);data.writeUInt16LE(2,22);data.writeUInt32LE(sr,24);data.writeUInt32LE(sr*4,28);data.writeUInt16LE(4,32);data.writeUInt16LE(16,34);data.write("data",36);data.writeUInt32LE(data.length-44,40);
  for(let i=0;i<buf[0].length;i++)for(let c=0;c<2;c++)data.writeInt16LE(Math.round(Math.max(-1,Math.min(1,buf[c][i]*gain))*32767),44+i*4+c*2);
  writeFileSync(out+name+".wav",data);
  console.log(`${name}: ${(buf[0].length/sr).toFixed(2)}s, peak ${(20*Math.log10(peak)).toFixed(1)} dBFS, RMS ${(20*Math.log10(Math.sqrt(squares/(buf[0].length*2))*gain)).toFixed(1)} dBFS`);
}
const music=track(48); // Extra tail accommodates small editorial changes.
const chords=[[62,66,69,73],[59,62,66,69],[55,59,62,66],[57,61,64,66]];
for(let beat=0;beat<94;beat++){
  const t=beat*.5,chord=chords[Math.floor(beat/8)%4];
  // Soft four-on-floor kick, offbeat tick and sparse snare.
  if(t<40 || beat%2===0)add(music,t,.23,x=>Math.sin(tau*(48*x+7*(1-Math.exp(-x*24))))*Math.exp(-x*20),.24);
  add(music,t+.25,.07,x=>noise()*Math.exp(-x*65),.018,beat%2?.35:-.35);
  if(beat%4===2)add(music,t,.13,x=>(noise()*.5+Math.sin(tau*185*x)*.2)*Math.exp(-x*32),.065);
  if(beat%2===0)add(music,t,.65,x=>Math.sin(tau*hz(chord[0]-24)*x)*Math.exp(-x*5),.19);
  if(beat%4===0)chord.forEach((n,j)=>add(music,t,2.3,x=>(Math.sin(tau*hz(n)*x)+.18*Math.sin(tau*hz(n)*2*x))*Math.exp(-x*1.8),.075,(j-1.5)*.28));
  if(beat%2===0){const n=chord[(Math.floor(beat/2)+1)%4]+12;
    const pluck=x=>(Math.sin(tau*hz(n)*x)+.2*Math.sin(tau*hz(n)*3*x))*Math.exp(-x*8);
    add(music,t+.125,.7,pluck,.09,-.2);add(music,t+.5,.55,pluck,.024,.5);
  }
}
save("music",music,.62);
const pop=track(.22);add(pop,0,.22,t=>Math.sin(tau*(620*t+32*(1-Math.exp(-t*18))))*Math.exp(-t*24),.8);save("pop",pop,.55);
const tick=track(.1);add(tick,0,.1,t=>(Math.sin(tau*1350*t)*.7+noise()*.15)*Math.exp(-t*65),.6);save("tick",tick,.5);
const swipe=track(.26);let low=0;add(swipe,0,.26,t=>{low=.88*low+.12*noise();return low*Math.sin(Math.PI*t/.26)**2;},1);save("swipe",swipe,.36);
const confirm=track(.65);[74,78,81].forEach((n,i)=>add(confirm,i*.065,.43,t=>Math.sin(tau*hz(n)*t)*Math.exp(-t*10),.28,(i-1)*.3));save("confirm",confirm,.6);
