//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:2.0.50727.1433
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

using Microsoft.Ccr.Core;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using System;
using System.Collections.Generic;
using System.Xml.Serialization;
using W3C.Soap;
using compression = System.IO.Compression;
using constructor = Microsoft.Dss.Services.Constructor;
using contractmanager = Microsoft.Dss.Services.ContractManager;
using contractmodel = Microsoft.Dss.Services.ContractModel;
using io = System.IO;
using pxsoarmsr = Robotics.SoarMSR.Proxy;
using reflection = System.Reflection;


namespace Robotics.SoarMSR.Proxy
{
    
    
    /// <summary>
    /// SoarMSR Contract
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public sealed class Contract
    {
        
        /// The Unique Contract Identifier for the SoarMSR service
        public const String Identifier = "http://schemas.tempuri.org/2008/03/soarmsrservice.html";
        
        /// The Dss Service dssModel Contract(s)
        public static List<contractmodel.ServiceSummary> ServiceModel()
        {
            contractmanager.ServiceSummaryLoader loader = new contractmanager.ServiceSummaryLoader();
            return loader.GetServiceSummaries(typeof(Contract).Assembly);

        }
        
        /// <summary>
        /// Creates an instance of the service associated with this contract
        /// </summary>
        /// <param name="contructorServicePort">Contractor Service that will create the instance</param>
        /// <param name="partners">Optional list of service partners for new service instance</param>
        /// <returns>Result PortSet for retrieving service creation response</returns>
        public static DsspResponsePort<CreateResponse> CreateService(constructor.ConstructorPort contructorServicePort, params PartnerType[] partners)
        {
            DsspResponsePort<CreateResponse> result = new DsspResponsePort<CreateResponse>();
            ServiceInfoType si = new ServiceInfoType(Contract.Identifier, null);
            if (partners != null)
            {
                si.PartnerList = new List<PartnerType>(partners);
            }
            Microsoft.Dss.Services.Constructor.Create create =
                new Microsoft.Dss.Services.Constructor.Create(si, result);
            contructorServicePort.Post(create);
            return result;

        }
        
        /// <summary>
        /// Creates an instance of the service associated with this contract
        /// </summary>
        /// <param name="contructorServicePort">Contractor Service that will create the instance</param>
        /// <returns>Result PortSet for retrieving service creation response</returns>
        public static DsspResponsePort<CreateResponse> CreateService(constructor.ConstructorPort contructorServicePort)
        {
            return Contract.CreateService(contructorServicePort, null);
        }
    }
    
    /// <summary>
    /// SoarMSR State
    /// </summary>
    [DataContract()]
    [XmlRootAttribute("SoarMSRState", Namespace="http://schemas.tempuri.org/2008/03/soarmsrservice.html")]
    public class SoarMSRState : ICloneable, IDssSerializable
    {
        
        private String _agentName;
        
        private String _productions;
        
        private Boolean _spawnDebugger;
        
        private Boolean _hasRandomSeed;
        
        private Int32 _randomSeed;
        
        private Double _drivePower;
        
        private Double _reversePower;
        
        private Int32 _stopTimeout;
        
        private Int32 _backUpTimeout;
        
        private Int32 _turnTimeout;
        
        private Int32 _timeoutVariance;
        
        private Int32 _obstacleAngleRange;
        
        private Int32 _minimumObstacleRange;
        
        private Boolean _obstacleRangeAverage;
        
        /// <summary>
        /// Agent Name
        /// </summary>
        [DataMember()]
        public String AgentName
        {
            get
            {
                return this._agentName;
            }
            set
            {
                this._agentName = value;
            }
        }
        
        /// <summary>
        /// Productions
        /// </summary>
        [DataMember()]
        public String Productions
        {
            get
            {
                return this._productions;
            }
            set
            {
                this._productions = value;
            }
        }
        
        /// <summary>
        /// Spawn Debugger
        /// </summary>
        [DataMember()]
        public Boolean SpawnDebugger
        {
            get
            {
                return this._spawnDebugger;
            }
            set
            {
                this._spawnDebugger = value;
            }
        }
        
        /// <summary>
        /// Has Random Seed
        /// </summary>
        [DataMember()]
        public Boolean HasRandomSeed
        {
            get
            {
                return this._hasRandomSeed;
            }
            set
            {
                this._hasRandomSeed = value;
            }
        }
        
        /// <summary>
        /// Random Seed
        /// </summary>
        [DataMember()]
        public Int32 RandomSeed
        {
            get
            {
                return this._randomSeed;
            }
            set
            {
                this._randomSeed = value;
            }
        }
        
        /// <summary>
        /// Drive Power
        /// </summary>
        [DataMember()]
        public Double DrivePower
        {
            get
            {
                return this._drivePower;
            }
            set
            {
                this._drivePower = value;
            }
        }
        
        /// <summary>
        /// Reverse Power
        /// </summary>
        [DataMember()]
        public Double ReversePower
        {
            get
            {
                return this._reversePower;
            }
            set
            {
                this._reversePower = value;
            }
        }
        
        /// <summary>
        /// Stop Timeout
        /// </summary>
        [DataMember()]
        public Int32 StopTimeout
        {
            get
            {
                return this._stopTimeout;
            }
            set
            {
                this._stopTimeout = value;
            }
        }
        
        /// <summary>
        /// Back Up Timeout
        /// </summary>
        [DataMember()]
        public Int32 BackUpTimeout
        {
            get
            {
                return this._backUpTimeout;
            }
            set
            {
                this._backUpTimeout = value;
            }
        }
        
        /// <summary>
        /// Turn Timeout
        /// </summary>
        [DataMember()]
        public Int32 TurnTimeout
        {
            get
            {
                return this._turnTimeout;
            }
            set
            {
                this._turnTimeout = value;
            }
        }
        
        /// <summary>
        /// Timeout Variance
        /// </summary>
        [DataMember()]
        public Int32 TimeoutVariance
        {
            get
            {
                return this._timeoutVariance;
            }
            set
            {
                this._timeoutVariance = value;
            }
        }
        
        /// <summary>
        /// Obstacle Angle Range
        /// </summary>
        [DataMember()]
        public Int32 ObstacleAngleRange
        {
            get
            {
                return this._obstacleAngleRange;
            }
            set
            {
                this._obstacleAngleRange = value;
            }
        }
        
        /// <summary>
        /// Minimum Obstacle Range
        /// </summary>
        [DataMember()]
        public Int32 MinimumObstacleRange
        {
            get
            {
                return this._minimumObstacleRange;
            }
            set
            {
                this._minimumObstacleRange = value;
            }
        }
        
        /// <summary>
        /// Obstacle Range Average
        /// </summary>
        [DataMember()]
        public Boolean ObstacleRangeAverage
        {
            get
            {
                return this._obstacleRangeAverage;
            }
            set
            {
                this._obstacleRangeAverage = value;
            }
        }
        
        /// <summary>
        /// Copy To SoarMSR State
        /// </summary>
        public virtual void CopyTo(IDssSerializable target)
        {
            SoarMSRState typedTarget = target as SoarMSRState;

            if (typedTarget == null)
                throw new ArgumentException("CopyTo({0}) requires type {0}", this.GetType().FullName);
            typedTarget.AgentName = this.AgentName;
            typedTarget.Productions = this.Productions;
            typedTarget.SpawnDebugger = this.SpawnDebugger;
            typedTarget.HasRandomSeed = this.HasRandomSeed;
            typedTarget.RandomSeed = this.RandomSeed;
            typedTarget.DrivePower = this.DrivePower;
            typedTarget.ReversePower = this.ReversePower;
            typedTarget.StopTimeout = this.StopTimeout;
            typedTarget.BackUpTimeout = this.BackUpTimeout;
            typedTarget.TurnTimeout = this.TurnTimeout;
            typedTarget.TimeoutVariance = this.TimeoutVariance;
            typedTarget.ObstacleAngleRange = this.ObstacleAngleRange;
            typedTarget.MinimumObstacleRange = this.MinimumObstacleRange;
            typedTarget.ObstacleRangeAverage = this.ObstacleRangeAverage;
        }
        
        /// <summary>
        /// Clone SoarMSR State
        /// </summary>
        public virtual object Clone()
        {
            SoarMSRState target = new SoarMSRState();

            target.AgentName = this.AgentName;
            target.Productions = this.Productions;
            target.SpawnDebugger = this.SpawnDebugger;
            target.HasRandomSeed = this.HasRandomSeed;
            target.RandomSeed = this.RandomSeed;
            target.DrivePower = this.DrivePower;
            target.ReversePower = this.ReversePower;
            target.StopTimeout = this.StopTimeout;
            target.BackUpTimeout = this.BackUpTimeout;
            target.TurnTimeout = this.TurnTimeout;
            target.TimeoutVariance = this.TimeoutVariance;
            target.ObstacleAngleRange = this.ObstacleAngleRange;
            target.MinimumObstacleRange = this.MinimumObstacleRange;
            target.ObstacleRangeAverage = this.ObstacleRangeAverage;
            return target;

        }
        
        /// <summary>
        /// Serialize Serialize
        /// </summary>
        public virtual void Serialize(System.IO.BinaryWriter writer)
        {
            if (AgentName == null) writer.Write((byte)0);
            else
            {
                // null flag
                writer.Write((byte)1);

                writer.Write(AgentName);
            }

            if (Productions == null) writer.Write((byte)0);
            else
            {
                // null flag
                writer.Write((byte)1);

                writer.Write(Productions);
            }

            writer.Write(SpawnDebugger);

            writer.Write(HasRandomSeed);

            writer.Write(RandomSeed);

            writer.Write(DrivePower);

            writer.Write(ReversePower);

            writer.Write(StopTimeout);

            writer.Write(BackUpTimeout);

            writer.Write(TurnTimeout);

            writer.Write(TimeoutVariance);

            writer.Write(ObstacleAngleRange);

            writer.Write(MinimumObstacleRange);

            writer.Write(ObstacleRangeAverage);

        }
        
        /// <summary>
        /// Deserialize Deserialize
        /// </summary>
        public virtual object Deserialize(System.IO.BinaryReader reader)
        {
            if (reader.ReadByte() == 0) {}
            else
            {
                AgentName = reader.ReadString();
            } //nullable

            if (reader.ReadByte() == 0) {}
            else
            {
                Productions = reader.ReadString();
            } //nullable

            SpawnDebugger = reader.ReadBoolean();

            HasRandomSeed = reader.ReadBoolean();

            RandomSeed = reader.ReadInt32();

            DrivePower = reader.ReadDouble();

            ReversePower = reader.ReadDouble();

            StopTimeout = reader.ReadInt32();

            BackUpTimeout = reader.ReadInt32();

            TurnTimeout = reader.ReadInt32();

            TimeoutVariance = reader.ReadInt32();

            ObstacleAngleRange = reader.ReadInt32();

            MinimumObstacleRange = reader.ReadInt32();

            ObstacleRangeAverage = reader.ReadBoolean();

            return this;

        }
    }
    
    /// <summary>
    /// SoarMSR Operations
    /// </summary>
    [ServicePort()]
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class SoarMSROperations : PortSet<Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup, Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop, Get, Replace>
    {
        
        /// <summary>
        /// Required Lookup request body type
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.LookupResponse,Fault> DsspDefaultLookup()
        {
            Microsoft.Dss.ServiceModel.Dssp.LookupRequestType body = new Microsoft.Dss.ServiceModel.Dssp.LookupRequestType();
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Dssp Default Lookup and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.LookupResponse,Fault> DsspDefaultLookup(Microsoft.Dss.ServiceModel.Dssp.LookupRequestType body)
        {
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.LookupRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// A request to drop the service.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultDropResponseType,Fault> DsspDefaultDrop()
        {
            Microsoft.Dss.ServiceModel.Dssp.DropRequestType body = new Microsoft.Dss.ServiceModel.Dssp.DropRequestType();
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Dssp Default Drop and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultDropResponseType,Fault> DsspDefaultDrop(Microsoft.Dss.ServiceModel.Dssp.DropRequestType body)
        {
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.DropRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Required Get body type
        /// </summary>
        public virtual PortSet<SoarMSRState,Fault> Get()
        {
            Microsoft.Dss.ServiceModel.Dssp.GetRequestType body = new Microsoft.Dss.ServiceModel.Dssp.GetRequestType();
            Get op = new Get(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Get and return the response port.
        /// </summary>
        public virtual PortSet<SoarMSRState,Fault> Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body)
        {
            Get op = new Get();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.GetRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// SoarMSR State
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType,Fault> Replace()
        {
            SoarMSRState body = new SoarMSRState();
            Replace op = new Replace(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Replace and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType,Fault> Replace(SoarMSRState body)
        {
            Replace op = new Replace();
            op.Body = body ?? new SoarMSRState();
            this.Post(op);
            return op.ResponsePort;

        }
    }
    
    /// <summary>
    /// Get
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class Get : Microsoft.Dss.ServiceModel.Dssp.Get<Microsoft.Dss.ServiceModel.Dssp.GetRequestType, PortSet<SoarMSRState, Fault>>
    {
        
        /// <summary>
        /// Get
        /// </summary>
        public Get()
        {
        }
        
        /// <summary>
        /// Get
        /// </summary>
        public Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body) : 
                base(body)
        {
        }
        
        /// <summary>
        /// Get
        /// </summary>
        public Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body, Microsoft.Ccr.Core.PortSet<SoarMSRState,W3C.Soap.Fault> responsePort) : 
                base(body, responsePort)
        {
        }
    }
    
    /// <summary>
    /// Replace
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class Replace : Microsoft.Dss.ServiceModel.Dssp.Replace<SoarMSRState, PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType, Fault>>
    {
        
        /// <summary>
        /// Replace
        /// </summary>
        public Replace()
        {
        }
        
        /// <summary>
        /// Replace
        /// </summary>
        public Replace(SoarMSRState body) : 
                base(body)
        {
        }
        
        /// <summary>
        /// Replace
        /// </summary>
        public Replace(SoarMSRState body, Microsoft.Ccr.Core.PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType,W3C.Soap.Fault> responsePort) : 
                base(body, responsePort)
        {
        }
    }
}
