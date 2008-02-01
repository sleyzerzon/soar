//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:2.0.50727.832
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
using compression = System.IO.Compression;
using constructor = Microsoft.Dss.Services.Constructor;
using contractmanager = Microsoft.Dss.Services.ContractManager;
using contractmodel = Microsoft.Dss.Services.ContractModel;
using dssphttp = Microsoft.Dss.Core.DsspHttp;
using io = System.IO;
using pxengine = Microsoft.Robotics.Simulation.Engine.Proxy;
using pxsimulationtutorial2 = Robotics.SimulationTutorial2.Proxy;
using reflection = System.Reflection;


namespace Robotics.SimulationTutorial2.Proxy
{
    
    
    /// <summary>
    /// SimulationTutorial2 Contract
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public sealed class Contract
    {
        
        /// The Unique Contract Identifier for the SimulationTutorial2 service
        public const String Identifier = "http://schemas.tempuri.org/2006/06/simulationtutorial2.html";
        
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
    /// State
    /// </summary>
    [DataContract()]
    [XmlRootAttribute("State", Namespace="http://schemas.tempuri.org/2006/06/simulationtutorial2.html")]
    public class State : ICloneable, IDssSerializable
    {
        
        private String _message;
        
        private Int32 _tableCount;
        
        /// <summary>
        /// Message
        /// </summary>
        [DataMember()]
        public String Message
        {
            get
            {
                return this._message;
            }
            set
            {
                this._message = value;
            }
        }
        
        /// <summary>
        /// Table Count
        /// </summary>
        [DataMember()]
        public Int32 TableCount
        {
            get
            {
                return this._tableCount;
            }
            set
            {
                this._tableCount = value;
            }
        }
        
        /// <summary>
        /// Copy To State
        /// </summary>
        public virtual void CopyTo(IDssSerializable target)
        {
            State typedTarget = target as State;

            if (typedTarget == null)
                throw new ArgumentException("CopyTo({0}) requires type {0}", this.GetType().FullName);
            typedTarget.Message = this.Message;
            typedTarget.TableCount = this.TableCount;
        }
        
        /// <summary>
        /// Clone State
        /// </summary>
        public virtual object Clone()
        {
            State target = new State();

            target.Message = this.Message;
            target.TableCount = this.TableCount;
            return target;

        }
        
        /// <summary>
        /// Serialize Serialize
        /// </summary>
        public virtual void Serialize(System.IO.BinaryWriter writer)
        {
            if (Message == null) writer.Write((byte)0);
            else
            {
                // null flag
                writer.Write((byte)1);

                writer.Write(Message);
            }

            writer.Write(TableCount);

        }
        
        /// <summary>
        /// Deserialize Deserialize
        /// </summary>
        public virtual object Deserialize(System.IO.BinaryReader reader)
        {
            if (reader.ReadByte() == 0) {}
            else
            {
                Message = reader.ReadString();
            } //nullable

            TableCount = reader.ReadInt32();

            return this;

        }
    }
    
    /// <summary>
    /// Table Entity
    /// </summary>
    [DataContract()]
    [XmlRootAttribute("TableEntity", Namespace="http://schemas.tempuri.org/2006/06/simulationtutorial2.html")]
    public class TableEntity : pxengine.MultiShapeEntity
    {
        
        /// <summary>
        /// Copy To Table Entity
        /// </summary>
        public override void CopyTo(IDssSerializable target)
        {
            TableEntity typedTarget = target as TableEntity;

            if (typedTarget == null)
                throw new ArgumentException("CopyTo({0}) requires type {0}", this.GetType().FullName);
            base.CopyTo(typedTarget);
        }
        
        /// <summary>
        /// Clone Table Entity
        /// </summary>
        public override object Clone()
        {
            TableEntity target = new TableEntity();

            base.CopyTo(target);
            return target;

        }
        
        /// <summary>
        /// Serialize Serialize
        /// </summary>
        public override void Serialize(System.IO.BinaryWriter writer)
        {
            base.Serialize(writer);
        }
        
        /// <summary>
        /// Deserialize Deserialize
        /// </summary>
        public override object Deserialize(System.IO.BinaryReader reader)
        {
            base.Deserialize(reader);
            return this;

        }
    }
    
    /// <summary>
    /// Simulation Tutorial2 Operations
    /// </summary>
    [ServicePort()]
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class SimulationTutorial2Operations : PortSet<Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup, Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop, dssphttp.HttpGet, Replace>
    {
        
        /// <summary>
        /// Required Lookup request body type
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.LookupResponse,W3C.Soap.Fault> DsspDefaultLookup()
        {
            Microsoft.Dss.ServiceModel.Dssp.LookupRequestType body = new Microsoft.Dss.ServiceModel.Dssp.LookupRequestType();
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Dssp Default Lookup and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.LookupResponse,W3C.Soap.Fault> DsspDefaultLookup(Microsoft.Dss.ServiceModel.Dssp.LookupRequestType body)
        {
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.LookupRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// A request to drop the service.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultDropResponseType,W3C.Soap.Fault> DsspDefaultDrop()
        {
            Microsoft.Dss.ServiceModel.Dssp.DropRequestType body = new Microsoft.Dss.ServiceModel.Dssp.DropRequestType();
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Dssp Default Drop and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultDropResponseType,W3C.Soap.Fault> DsspDefaultDrop(Microsoft.Dss.ServiceModel.Dssp.DropRequestType body)
        {
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.DropRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// DsspHttp Get request body
        /// </summary>
        public virtual PortSet<dssphttp.HttpResponseType,W3C.Soap.Fault> HttpGet()
        {
            dssphttp.HttpGetRequestType body = new dssphttp.HttpGetRequestType();
            dssphttp.HttpGet op = new dssphttp.HttpGet(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Http Get and return the response port.
        /// </summary>
        public virtual PortSet<dssphttp.HttpResponseType,W3C.Soap.Fault> HttpGet(dssphttp.HttpGetRequestType body)
        {
            dssphttp.HttpGet op = new dssphttp.HttpGet();
            op.Body = body ?? new dssphttp.HttpGetRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// State
        /// </summary>
        public virtual Microsoft.Dss.ServiceModel.Dssp.DsspResponsePort<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType> Replace()
        {
            State body = new State();
            Replace op = new Replace(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Replace and return the response port.
        /// </summary>
        public virtual Microsoft.Dss.ServiceModel.Dssp.DsspResponsePort<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType> Replace(State body)
        {
            Replace op = new Replace();
            op.Body = body ?? new State();
            this.Post(op);
            return op.ResponsePort;

        }
    }
    
    /// <summary>
    /// Replace
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class Replace : Microsoft.Dss.ServiceModel.Dssp.Replace<State, Microsoft.Dss.ServiceModel.Dssp.DsspResponsePort<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType>>
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
        public Replace(State body) : 
                base(body)
        {
        }
        
        /// <summary>
        /// Replace
        /// </summary>
        public Replace(State body, Microsoft.Dss.ServiceModel.Dssp.DsspResponsePort<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType> responsePort) : 
                base(body, responsePort)
        {
        }
    }
}
